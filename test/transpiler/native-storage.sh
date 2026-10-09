#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
if [[ -z "${AGIRU_SYSTEM_SYMBOLS:-}" ]]; then
  printf 'native-storage: explicit verified AGIRU_SYSTEM_SYMBOLS is required\n' >&2
  exit 2
fi
package=$(realpath "$AGIRU_SYSTEM_SYMBOLS")
python3 scripts/fetch_symbols.py --verify "$package"
proof=$(mktemp -d /tmp/agiru-native-storage.XXXXXX)
printf '%s\n' "$proof" > "$B/native-storage.latest"
input="$PWD/test/transpiler/native-storage"
cp -a "$input" "$proof/input"
mkdir -p "$proof/input/package/src"
cp "$package/NavxManifest.xml" "$proof/input/package/"
originals=(AccessControl TenantPermissionSet TenantPermission TenantPermissionSetRel EntityText)
for source in "${originals[@]}"; do
  path="$package/src/Tenant Database Tables/$source.Table.al"
  sha256sum "$path" >> "$proof/originals.sha256"
  cp "$path" "$proof/input/package/src/"
done
enum_source="$package/src/System Enums/EntityTextScenario.Enum.al"
sha256sum "$enum_source" >> "$proof/originals.sha256"
cp "$enum_source" "$proof/input/package/src/"
sha256sum "$package/System.app" "$package/NavxManifest.xml" \
  "$package/SymbolReference.json" >> "$proof/originals.sha256"
cp "$package/provenance.json" "$proof/package-provenance.json"
sha256sum "$B/agirutc" "$B/libagiru_gen.so" "$B/libagiru_rt.so" > "$proof/binaries.sha256"
generate() {
  local root=$1
  "$B/agirutc" "$root" "$root/apps.json" "$root/generated" \
    --system-symbols "$root/package" --host-runtime 18.0 > "$root/generation.log" 2>&1
  rg -q '^native 5 table sources parsed, 5 bound, 0 unbound, 0 source refusals;' "$root/generation.log"
  [[ $(rg --files --no-ignore "$root/generated/platform" -g '*.def.cpp' | wc -l) -eq 5 ]]
}
generate "$proof/input"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-DAGIRU_TEST_DSN=\"${AGIRU_TEST_DSN:-postgresql://agiru:agiru@127.0.0.1:5432/agiru_gate}\"")
links=(--rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
compile() {
  local root=$1
  local includes=("-I$root/generated/platform" "-I$root/generated/shared"
    "-I$root/generated/absent" "-I$root/generated/fixture")
  local sources=()
  mapfile -t sources < <(rg --files --no-ignore "$root/generated" -g '*.cpp' | LC_ALL=C sort)
  "$CXX" "${flags[@]}" "${includes[@]}" -c "$input/Runner.cpp" -o "$root/runner.o"
  "$CXX" "${flags[@]}" "${includes[@]}" "$root/runner.o" "${sources[@]}" \
    "${links[@]}" -o "$root/runner"
}
compile "$proof/input"
"$proof/input/runner" | tee "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$input/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" "-I$proof/input/generated/platform" \
  "-I$proof/input/generated/shared" "-I$proof/input/generated/absent" \
  "-I$proof/input/generated/fixture" -c "$input/Runner.cpp" -o "$proof/input/runner.o" \
  > "$B/fixture-commands/native-storage.json"
for control in init-value role-width entity-width; do
  cp -a "$input" "$proof/$control"
  cp -a "$proof/input/package" "$proof/$control/package"
  changed="$proof/$control/package/src/TenantPermissionSet.Table.al"
  if [[ "$control" = init-value ]]; then
    sed -i 's/InitValue = true;/InitValue = false;/' "$changed"
  elif [[ "$control" = role-width ]]; then
    sed -i 's/Code\[20\]/Code[21]/' "$changed"
  else
    changed="$proof/$control/package/src/EntityText.Table.al"
    sed -i 's/Text\[1024\]/Text[1023]/' "$changed"
  fi
  ! cmp -s "$changed" "$proof/input/package/src/$(basename "$changed")"
  generate "$proof/$control"
  compile "$proof/$control"
  if "$proof/$control/runner" > "$proof/$control-execution.log" 2>&1; then
    printf 'native-storage: %s escaped SQL/source qualification\n' "$control" >&2
    exit 1
  fi
  rg -q '^FAIL ' "$proof/$control-execution.log"
done
sha256sum --check --status "$proof/originals.sha256"
rm -f "$proof/input/runner" "$proof/input/runner.o" \
  "$proof/init-value/runner" "$proof/init-value/runner.o" \
  "$proof/role-width/runner" "$proof/role-width/runner.o"
rm -f "$proof/entity-width/runner" "$proof/entity-width/runner.o"
printf 'native-storage: five original stored tables; typed SQL rights and three source controls; %s\n' "$proof"
