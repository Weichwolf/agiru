#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
input="$PWD/test/transpiler/native-table-ids"
proof=$(mktemp -d /tmp/agiru-native-table-ids.XXXXXX)
printf 'native-table-ids: receipts %s\n' "$proof"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate)
links=(--rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
sha256sum src/al/Statements.cpp src/gen/{BodyWriter,CodeunitWriter}.{cpp,h} src/tc/Main.cpp \
  "$B/agirutc" "$B/libagiru_gen.so" "$B/libagiru_al.so" > "$proof/inputs.sha256"
find "$input" -type f -exec sha256sum {} + >> "$proof/inputs.sha256"

generate() {
  local root=$1 output=$2 expected=$3 identity=${4:-Declared Only} status=0
  "$B/agirutc" "$root" "$root/apps.json" "$output" --system-symbols "$root/package" \
    > "$output.log" 2>&1 || status=$?
  [ "$status" -eq "$expected" ] || { printf 'native-table-ids: unexpected translation exit %s\n' "$status" >&2; exit 1; }
  if [ "$expected" -eq 1 ]; then
    rg -q '^native 1 table sources parsed, 0 bound, 1 unbound, 0 source refusals;' "$output.log"
    rg -q "^native-unbound [0-9]+ $identity:" "$output.log"
  fi
  if rg -q '^(FAIL|refused +[1-9]|unplaced +[1-9])' "$output.log"; then
    printf 'native-table-ids: unrelated translation failure\n' >&2
    exit 1
  fi
}

link_generated() {
  local output=$1 runner=$2
  local sources=()
  mapfile -t sources < <(rg --files --no-ignore "$output" -g '*.cpp' | LC_ALL=C sort)
  [ "${#sources[@]}" -gt 0 ]
  "$CXX" "${flags[@]}" "-I$output/fixture" "-I$output/shared" "-I$output/absent" \
    "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$runner"
}

generate "$input" "$proof/generated" 1
"$CXX" "${flags[@]}" "-I$proof/generated/fixture" "-I$proof/generated/shared" \
  "-I$proof/generated/absent" -c "$input/Runner.cpp" -o "$proof/runner.o"
link_generated "$proof/generated" "$proof/runner"
"$proof/runner" | tee "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$input/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" "-I$proof/generated/fixture" "-I$proof/generated/shared" \
  "-I$proof/generated/absent" -c "$input/Runner.cpp" -o "$proof/runner.o" \
  > "$B/fixture-commands/native-table-ids.json"

for control in source-id missing excluded; do
  cp -a "$input" "$proof/$control"
  declared="$proof/$control/package/src/DeclaredOnly.Table.al"
  expected=0
  identity='Declared Only'
  case "$control" in
    source-id)
      sed -i 's/table 2000000821/table 2000000822/' "$declared"
      ! cmp -s "$input/package/src/DeclaredOnly.Table.al" "$declared"
      expected=1 ;;
    missing)
      sed 's/"Declared Only"/"Unrelated Table"/' "$declared" \
        > "$proof/$control/package/src/Unrelated.Table.al"
      rm "$declared"
      expected=1
      identity='Unrelated Table' ;;
    excluded)
      jq '.product_exclude=["bc-licensing:system-symbols/src/DeclaredOnly.Table.al"]' \
        "$input/scope.json" > "$proof/$control/scope.json" ;;
  esac
  generate "$proof/$control" "$proof/$control/generated" "$expected" "$identity"
  link_generated "$proof/$control/generated" "$proof/$control/runner"
  if "$proof/$control/runner" > "$proof/$control-execution.log" 2>&1; then
    printf 'native-table-ids: %s escaped source-identity execution\n' "$control" >&2
    exit 1
  fi
  if [ "$control" = source-id ]; then
    [ "$(rg -c '^FAIL ' "$proof/$control-execution.log")" -eq 8 ]
  else
    rg -q 'Database::Declared Only' "$proof/$control-execution.log"
  fi
done
for control in duplicate-id duplicate-name app-collision; do
  cp -a "$input" "$proof/$control"
  case "$control" in
    duplicate-id)
      sed 's/"Declared Only"/"Other Table"/' "$input/package/src/DeclaredOnly.Table.al" \
        > "$proof/$control/package/src/Duplicate.Table.al" ;;
    duplicate-name)
      sed 's/table 2000000821/table 2000000822/' "$input/package/src/DeclaredOnly.Table.al" \
        > "$proof/$control/package/src/Duplicate.Table.al" ;;
    app-collision)
      sed -i 's/table 50302/table 2000000821/' "$proof/$control/source/IDOwner.Table.al" ;;
  esac
  if "$B/agirutc" "$proof/$control" "$proof/$control/apps.json" \
    --system-symbols "$proof/$control/package" > "$proof/$control.log" 2>&1; then
    printf 'native-table-ids: %s escaped identity validation\n' "$control" >&2
    exit 1
  fi
  rg -q 'duplicate System table declaration|AL table duplicates declared System table ID' "$proof/$control.log"
done
rm -f "$proof/runner.o" "$proof/runner" "$proof"/{source-id,missing,excluded}/runner
printf 'native-table-ids: four object writers execute; six identity controls refuse; native table stays unbound; %s\n' "$proof"
