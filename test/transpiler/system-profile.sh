#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-system-profile.XXXXXX)
input="$PWD/test/transpiler/system-profile"
trap 'find "$proof" -maxdepth 1 -type f \( -name "*.o" -o -name "runner-*" \) -delete' EXIT
sha256sum "$B/agirutc" "$input"/{apps.json,scope.json,Consumer.cpp} \
  "$input/al/fixture/"* > "$proof/inputs.sha256"
cp -a "$input/al" "$proof/source"
cp "$input/"{apps.json,scope.json} "$proof/"
base_flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate)
links=(--rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
mkdir -p "$B/fixture-commands"
for runtime in 17.0 18.0; do
  for kind in Normal Temporary CRM CDS ExternalSQL Exchange MicrosoftGraph; do
    for linked in false true; do
      name="$runtime-$kind-$linked"
      output="$proof/$name"
      awk -v kind="$kind" -v linked="$linked" '
        /TableType = Normal;/ { sub(/Normal/, kind); kinds++ }
        /LinkedObject = false;/ { sub(/false/, linked); links++ }
        { print }
        END { if (kinds != 1 || links != 1) exit 2 }
      ' "$input/al/fixture/ProfileRow.Table.al" > "$proof/source/fixture/ProfileRow.Table.al"
      status=0
      "$B/agirutc" "$proof/source" "$proof/apps.json" "$output" --host-runtime "$runtime" \
        > "$proof/$name-generation.log" 2>&1 || status=$?
      if [ "$linked" = true ] || { [ "$kind" != Normal ] && [ "$kind" != Temporary ]; }; then
        [ "$status" -eq 1 ]
        if [ "$linked" = true ]; then rg -q 'LinkedObject in table ProfileRow' "$proof/$name-generation.log"; fi
        if [ "$kind" != Normal ] && [ "$kind" != Temporary ]; then
          rg -q 'TableType in table ProfileRow' "$proof/$name-generation.log"
        fi
      else [ "$status" -eq 0 ]; fi
      rg -q '0 of 0 kind\(s\) read and dropped in silence' "$proof/$name-generation.log"
      audit=0
      lookup=0
      count=4
      if [ "$linked" = false ] && { [ "$kind" = Normal ] || [ "$kind" = Temporary ]; }; then
        audit=1
        count=8
        if [ "$runtime" = 18.0 ]; then lookup=1; count=12; fi
      fi
      flags=("${base_flags[@]}" "-I$output/fixture" "-I$output/shared" "-I$output/absent"
        "-DAGIRU_TEST_DSN=\"$dsn\"" "-DAGIRU_PROFILE_FIELDS=$count"
        "-DAGIRU_PROFILE_AUDIT=$audit" "-DAGIRU_PROFILE_LOOKUP=$lookup")
      mapfile -t sources < <(rg --files --no-ignore "$output" -g '*.cpp' | LC_ALL=C sort)
      [ "${#sources[@]}" -gt 0 ]
      "$CXX" "${flags[@]}" -c "$input/Consumer.cpp" -o "$proof/$name.o" \
        > "$proof/$name-consumer.log" 2>&1
      "$CXX" "${flags[@]}" "$proof/$name.o" "${sources[@]}" "${links[@]}" \
        -o "$proof/runner-$name" > "$proof/$name-link.log" 2>&1
      jq -n --arg directory "$PWD" --arg file "$input/Consumer.cpp" \
        --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
        "$CXX" "${flags[@]}" -c "$input/Consumer.cpp" -o "$proof/$name.o" \
        > "$B/fixture-commands/system-profile-$name.json"
      "$proof/runner-$name" --declarations > "$proof/$name-declarations.log" 2>&1
      if [ "$kind" = Normal ] && [ "$linked" = false ]; then
        "$proof/runner-$name" > "$proof/$name-execution.log" 2>&1
        cat "$proof/$name-execution.log"
      fi
    done
  done
done

cp "$input/al/fixture/ProfileRow.Table.al" "$proof/source/fixture/ProfileRow.Table.al"
for host in default 17.0; do
  output="$proof/wrapper-$host"
  environment=(env -u AGIRU_SYSTEM_SYMBOLS -u AGIRU_HOST_RUNTIME B="$B")
  count=12
  lookup=1
  if [ "$host" = 17.0 ]; then environment+=(AGIRU_HOST_RUNTIME=17.0); count=8; lookup=0; fi
  "${environment[@]}" bash scripts/transpile.sh "$proof/source" "$proof/apps.json" "$output" \
    > "$proof/wrapper-$host-generation.log" 2>&1
  flags=("${base_flags[@]}" "-I$output/fixture" "-I$output/shared" "-I$output/absent"
    "-DAGIRU_TEST_DSN=\"$dsn\"" "-DAGIRU_PROFILE_FIELDS=$count"
    -DAGIRU_PROFILE_AUDIT=1 "-DAGIRU_PROFILE_LOOKUP=$lookup")
  "$CXX" "${flags[@]}" -c "$input/Consumer.cpp" -o "$proof/wrapper-$host.o"
  mapfile -t sources < <(rg --files --no-ignore "$output" -g '*.cpp' | LC_ALL=C sort)
  [ "${#sources[@]}" -gt 0 ]
  "$CXX" "${flags[@]}" "$proof/wrapper-$host.o" "${sources[@]}" "${links[@]}" \
    -o "$proof/runner-wrapper-$host"
  jq -n --arg directory "$PWD" --arg file "$input/Consumer.cpp" \
    --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
    "$CXX" "${flags[@]}" -c "$input/Consumer.cpp" -o "$proof/wrapper-$host.o" \
    > "$B/fixture-commands/system-profile-wrapper-$host.json"
  "$proof/runner-wrapper-$host" > "$proof/wrapper-$host-execution.log" 2>&1
  cat "$proof/wrapper-$host-execution.log"
done
status=0
env -u AGIRU_SYSTEM_SYMBOLS B="$B" AGIRU_HOST_RUNTIME= bash scripts/transpile.sh \
  "$proof/source" "$proof/apps.json" "$proof/wrapper-empty" \
  > "$proof/wrapper-empty.log" 2>&1 || status=$?
[ "$status" -eq 1 ]
rg -q 'unsupported host runtime' "$proof/wrapper-empty.log"
[ ! -e "$proof/wrapper-empty" ]

output="$proof/18.0-Normal-false"
flags=("${base_flags[@]}" "-I$output/fixture" "-I$output/shared" "-I$output/absent"
  "-DAGIRU_TEST_DSN=\"$dsn\"" -DAGIRU_PROFILE_FIELDS=12
  -DAGIRU_PROFILE_AUDIT=1 -DAGIRU_PROFILE_LOOKUP=1)
for control in audit lookup; do
  altered=("${flags[@]}")
  if [ "$control" = audit ]; then altered+=(-UAGIRU_PROFILE_AUDIT -DAGIRU_PROFILE_AUDIT=0);
  else altered+=(-UAGIRU_PROFILE_LOOKUP -DAGIRU_PROFILE_LOOKUP=0); fi
  if "$CXX" "${altered[@]}" -fsyntax-only "$input/Consumer.cpp" \
    > "$proof/$control-control.log" 2>&1; then
    printf 'system-profile: incorrect %s presence escaped the consumer\n' "$control" >&2
    exit 1
  fi
  rg -q 'static assertion failed' "$proof/$control-control.log"
done
awk '
  /WithImplicitFields<.*SystemFieldProfile::Runtime18/ {
    sub(/SystemFieldProfile::Runtime18/, "SystemFieldProfile::Runtime17"); changed++
  }
  { print }
  END { if (changed != 1) exit 2 }
' "$output/fixture/fixture/table/ProfileRow.def.cpp" > "$proof/profile-control.cpp"
if "$CXX" "${flags[@]}" "-I$output/fixture/fixture/table" -fsyntax-only "$proof/profile-control.cpp" \
  > "$proof/profile-control.log" 2>&1; then
  printf 'system-profile: a downgraded producer escaped its emitted field-count contract\n' >&2
  exit 1
fi
rg -q 'static assertion failed' "$proof/profile-control.log"
for invalid in 16.0 19.0 invalid; do
  status=0
  "$B/agirutc" "$proof/source" "$proof/apps.json" "$proof/invalid-$invalid" \
    --host-runtime "$invalid" > "$proof/invalid-$invalid.log" 2>&1 || status=$?
  [ "$status" -eq 1 ]
  rg -q 'unsupported host runtime' "$proof/invalid-$invalid.log"
  [ ! -e "$proof/invalid-$invalid" ]
done
status=0
"$B/agirutc" "$proof/source" "$proof/apps.json" "$proof/duplicate" \
  --host-runtime 17.0 --host-runtime 18.0 > "$proof/duplicate.log" 2>&1 || status=$?
[ "$status" -eq 1 ]
rg -q 'duplicate --host-runtime' "$proof/duplicate.log"
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
printf 'system-profile: 28 host/kind/linked declarations and two production-wrapper profiles qualify; generated AL executes both hosts; presence/profile/options/empty-host controls refuse; %s\n' "$proof"
