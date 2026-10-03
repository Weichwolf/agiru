#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-table-keys.XXXXXX)
input="$PWD/test/transpiler/table-keys"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)

generate() {
  local compiler=$1 output=$2
  "$compiler" "$input/al" "$input/apps.json" "$output" > "$output.generation.log" 2>&1
}

compile() {
  local generated=$1 output=$2
  local sources
  rg --files --no-ignore "$generated" -g '*.cpp' | LC_ALL=C sort > "$output.sources"
  mapfile -t sources < "$output.sources"
  [ "${#sources[@]}" -gt 0 ] || { printf 'table-keys: no generated sources\n' >&2; return 2; }
  "$CXX" "${flags[@]}" "-I$generated" "-I$generated/fixture" "-I$generated/shared" \
    -c test/transpiler/table-keys/Runner.cpp -o "$output.runner.o"
  "$CXX" "${flags[@]}" "-I$generated" "-I$generated/fixture" "-I$generated/shared" \
    "$output.runner.o" "${sources[@]}" "${links[@]}" -o "$output"
}

generate "$B/agirutc" "$proof/generated"
compile "$proof/generated" "$proof/runner"
"$proof/runner"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/transpiler/table-keys/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" "-I$proof/generated" "-I$proof/generated/fixture" \
  "-I$proof/generated/shared" -c test/transpiler/table-keys/Runner.cpp -o "$proof/runner.runner.o" \
  > "$proof/compile_commands.json"
cp "$proof/compile_commands.json" "$B/fixture-commands/table-keys.json"

for control in wrong-name wrong-clustering; do
  cp -a "$proof/generated" "$proof/$control"
  source="$proof/$control/fixture/fixture/table/ImplicitRow.def.cpp"
  if [ "$control" = wrong-name ]; then
    awk '
      /\.name = "Primary ID"/ { sub(/\.name = "Primary ID"/, ".name = \"primary id\""); changed++ }
      { print }
      END { if (changed != 1) exit 2 }
    ' "$source" > "$proof/$control.cpp"
  else
    awk '
      /\.name = "Primary ID"/ { selected=1 }
      selected && /\.clustered = true/ { sub(/\.clustered = true/, ".clustered = false"); changed++; selected=0 }
      { print }
      END { if (changed != 1) exit 2 }
    ' "$source" > "$proof/$control.cpp"
  fi
  mv "$proof/$control.cpp" "$source"
  compile "$proof/$control" "$proof/$control-runner" > "$proof/$control.compile.log" 2>&1
  if "$proof/$control-runner" > "$proof/$control.run.log" 2>&1; then
    printf 'table-keys: %s escaped the generated execution gate\n' "$control" >&2
    exit 1
  fi
  rg -q '1 red' "$proof/$control.run.log"
done

if [ -n "${AGIRU_KEYS_PREVIOUS:-}" ]; then
  LD_LIBRARY_PATH="$(dirname "$AGIRU_KEYS_PREVIOUS")" generate "$AGIRU_KEYS_PREVIOUS" "$proof/previous"
  compile "$proof/previous" "$proof/previous-runner" > "$proof/previous.compile.log" 2>&1
  if "$proof/previous-runner" > "$proof/previous.run.log" 2>&1; then
    printf 'table-keys: the previous emitter promoted an extension key without rejection\n' >&2
    exit 1
  fi
  rg -q 'extension keys never replace the implicit primary key' "$proof/previous.run.log"
  rg -q 'the primary key belongs to the base table' "$proof/previous.run.log"
fi
printf 'table-keys: original key identity and temporary operations pass; controls reject; %s\n' "$proof"
