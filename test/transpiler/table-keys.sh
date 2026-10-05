#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-table-keys.XXXXXX)
input="$PWD/test/transpiler/table-keys"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate -Isrc/rt)
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
  "$CXX" "${flags[@]}" "-I$generated" "-I$generated/fixture" "-I$generated/group" "-I$generated/orphan" "-I$generated/shared" \
    -c test/transpiler/table-keys/Runner.cpp -o "$output.runner.o"
  "$CXX" "${flags[@]}" "-I$generated" "-I$generated/fixture" "-I$generated/group" "-I$generated/orphan" "-I$generated/shared" \
    "$output.runner.o" "${sources[@]}" "${links[@]}" -o "$output"
}

repoint_takeover() {
  local changed=$1
  awk '
    /MovedFrom =/ {
      sub(/834a40c9-7a26-46f2-9348-3f6cc8c71719/, "118874ab-44bc-4ccb-9daf-59763539ab16"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "$changed" > "$changed.changed"
  mv "$changed.changed" "$changed"
}

generate "$B/agirutc" "$proof/generated"
compile "$proof/generated" "$proof/runner"
"$proof/runner"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/transpiler/table-keys/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" "-I$proof/generated" "-I$proof/generated/fixture" \
  "-I$proof/generated/group" "-I$proof/generated/orphan" "-I$proof/generated/shared" \
  -c test/transpiler/table-keys/Runner.cpp -o "$proof/runner.runner.o" \
  > "$proof/compile_commands.json"
cp "$proof/compile_commands.json" "$B/fixture-commands/table-keys.json"

for control in registry-missing registry-wrong-id; do
  awk -v control="$control" '
    /const auto \*entry = FindTable\(id\);/ {
      if (control == "registry-missing") {
        print "  static_cast<void>(id); return std::nullopt;"
      } else {
        print "  static_cast<void>(id);"
        sub(/FindTable\(id\)/, "FindTable(::agiru::Fixture::ComposedRow_Table::kId)")
      }
      changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/TableMetadata.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" -fPIC -shared "-I$proof/generated/fixture" \
    -include "$proof/generated/fixture/fixture/table/ComposedRow.h" \
    "$proof/$control.cpp" "${links[@]}" -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$proof/runner" > "$proof/$control.run.log" 2>&1; then
    printf 'table-keys: %s escaped installed metadata lookup\n' "$control" >&2
    exit 1
  fi
  rg -q 'installed lookup selects|installed lookup projects' "$proof/$control.run.log"
done

for control in wrong-name wrong-clustering wrong-owner wrong-classification wrong-customization-owner wrong-customization-override; do
  cp -a "$proof/generated" "$proof/$control"
  source="$proof/$control/fixture/fixture/table/ImplicitRow.def.cpp"
  if [ "$control" = wrong-name ]; then
    awk '
      /\.name = "Primary ID"/ { sub(/\.name = "Primary ID"/, ".name = \"primary id\""); changed++ }
      { print }
      END { if (changed != 1) exit 2 }
    ' "$source" > "$proof/$control.cpp"
  elif [ "$control" = wrong-clustering ]; then
    awk '
      /\.name = "Primary ID"/ { selected=1 }
      selected && /\.clustered = true/ { sub(/\.clustered = true/, ".clustered = false"); changed++; selected=0 }
      { print }
      END { if (changed != 1) exit 2 }
    ' "$source" > "$proof/$control.cpp"
  elif [ "$control" = wrong-owner ]; then
    source="$proof/$control/fixture/FixtureModule.h"
    awk '
      /118874ab-44bc-4ccb-9daf-59763539ab16/ {
        sub(/118874ab-44bc-4ccb-9daf-59763539ab16/, "85a884cd-20d8-4d18-91bd-e6c1baaa3a32"); changed++
      }
      { print }
      END { if (changed != 1) exit 2 }
    ' "$source" > "$proof/$control.cpp"
  elif [ "$control" = wrong-classification ]; then
    awk '
      /\.dataClassification = "AccountData"/ {
        sub(/AccountData/, "CustomerContent"); changed++
      }
      { print }
      END { if (changed != 1) exit 2 }
    ' "$source" > "$proof/$control.cpp"
  else
    expected=AsReadWrite
    if [ "$control" = wrong-customization-override ]; then expected=AsReadOnly; fi
    awk -v expected="$expected" '
      index($0, ".allowInCustomizations = \"" expected "\"") {
        sub(expected, "Never"); changed++
      }
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
  case "$control" in
    wrong-name) claim='the primary key retains the original AL field name';;
    wrong-clustering) claim='the primary key is clustered by default';;
    wrong-owner) claim='table owner comes from the source manifest';;
    wrong-classification) claim='table classification is retained';;
    wrong-customization-owner|wrong-customization-override) claim='generated customization retains the declaring owner or field override';;
  esac
  rg -q "FAIL .*${claim}" "$proof/$control.run.log"
done

cp -a "$proof/generated" "$proof/parent-owner"
source="$proof/parent-owner/group/fixture/table/OwnedRow.def.cpp"
awk '
  /^#include "SourceApp.*Module.h"/ { print "#include \"FixtureModule.h\"" }
  /\.module = &::agiru::app::SourceApp/ {
    sub(/::agiru::app::SourceApp[^:]+::kModule/, "::agiru::app::Fixture::kModule"); changed++
  }
  { print }
  END { if (changed != 1) exit 2 }
' "$source" > "$proof/parent-owner.cpp"
mv "$proof/parent-owner.cpp" "$source"
compile "$proof/parent-owner" "$proof/parent-owner-runner" > "$proof/parent-owner.compile.log" 2>&1
if "$proof/parent-owner-runner" > "$proof/parent-owner.run.log" 2>&1; then
  printf 'table-keys: parent owner escaped nested identity checks\n' >&2; exit 1
fi
rg -q 'nearest source manifest' "$proof/parent-owner.run.log"

for control in duplicate-app missing-identity dependency-owner symlink-manifest wrong-takeover; do
  cp -a "$input/al" "$proof/$control-input"
  manifest="$proof/$control-input/group/nested/app.json"
  if [ "$control" = symlink-manifest ]; then
    mv "$manifest" "$manifest.saved"
    ln -s missing-app.json "$manifest"
    expected='app.json is a symlink'
  else
    if [ "$control" = duplicate-app ]; then
      mkdir "$proof/$control-input/group/duplicate"
      cp "$manifest" "$proof/$control-input/group/duplicate/app.json"
      mv "$proof/$control-input/group/nested/SecondOwnedRow.Table.al" \
        "$proof/$control-input/group/duplicate/SecondOwnedRow.Table.al"
      cp "$manifest" "$manifest.changed"
      expected='duplicate table-owner app id'
    elif [ "$control" = missing-identity ]; then
      jq 'del(.publisher)' "$manifest" > "$manifest.changed"
      expected='app.json lacks nonempty string identity field publisher'
    elif [ "$control" = dependency-owner ]; then
      jq 'del(.id)' "$manifest" > "$manifest.changed"
      expected='app.json lacks nonempty string identity field id'
    else
      repoint_takeover "$proof/$control-input/fixture/OwnedRowExtension.TableExt.al"
      cp "$manifest" "$manifest.changed"
      expected='invalid moved field takeover'
    fi
    mv "$manifest.changed" "$manifest"
  fi
  if "$B/agirutc" "$proof/$control-input" "$input/apps.json" "$proof/$control-output" \
    > "$proof/$control-generation.log" 2>&1; then
    printf 'table-keys: %s escaped source identity validation\n' "$control" >&2; exit 1
  fi
  rg -q "$expected" "$proof/$control-generation.log"
done

if [ -n "${AGIRU_OWNER_PREVIOUS:-}" ]; then
  if "$AGIRU_OWNER_PREVIOUS" "$input/al" "$input/apps.json" "$proof/previous-owner" \
    > "$proof/previous-owner.generation.log" 2>&1; then
    printf 'table-keys: previous compiler accepted a nested source takeover\n' >&2; exit 1
  fi
  rg -q 'invalid moved field takeover' "$proof/previous-owner.generation.log"
  sha256sum "$AGIRU_OWNER_PREVIOUS" > "$proof/previous-owner.sha256"
fi

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
printf 'table-keys: root/nested/shared/missing owners, original keys and temporary operations pass; identity/ownership controls reject; %s\n' "$proof"
