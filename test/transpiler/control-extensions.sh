#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-control-extensions.XXXXXX)
printf 'control-extensions: receipts %s\n' "$proof"
mkdir -p "$proof/source" "$proof/extension"
cp test/transpiler/control-extensions/*.Page.al test/transpiler/control-extensions/*.Report.al "$proof/source/"
cp test/transpiler/control-extensions/*Ext.al "$proof/extension/"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
cp test/transpiler/control-extensions/ExtensionApp.json "$proof/extension/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"},{"name":"extension","source":"extension"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
sha256sum src/tc/Main.cpp test/transpiler/control-extensions/* "$B/agirutc" \
  "$B/libagiru_gen.so" "$B/libagiru_al.so" > "$proof/inputs.sha256"
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
if rg -q '^unplaced' "$proof/generation.log"; then
  printf 'control-extensions: declared forward anchors were not resolved\n' >&2
  exit 1
fi
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -gt 0 ]
"$CXX" "${flags[@]}" "-I$proof/generated/fixture" "-I$proof/generated/absent" \
  "-I$proof/generated/shared" -c test/transpiler/control-extensions/Runner.cpp -o "$proof/runner.o"
"$CXX" "${flags[@]}" "-I$proof/generated/fixture" "-I$proof/generated/absent" \
  "-I$proof/generated/shared" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/runner"
"$proof/runner" | tee "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/transpiler/control-extensions/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" "-I$proof/generated/fixture" "-I$proof/generated/absent" \
  "-I$proof/generated/shared" -c test/transpiler/control-extensions/Runner.cpp -o "$proof/runner.o" \
  > "$B/fixture-commands/control-extensions.json"

for control in missing-page-owner cpp-page-namespace lost-multiple-new-lines; do
  cp -a "$proof/generated" "$proof/$control"
  for source in "$proof/$control/fixture/fixture/page/ExtensionControls.def.cpp" \
    "$proof/$control/fixture/fixture/report/ExtensionRequestControls.def.cpp"; do
    awk -v control="$control" '
      control == "missing-page-owner" && /\.module =/ { changed++; next }
      control == "cpp-page-namespace" && /\.nameSpace = "Microsoft.Fixture"/ {
        sub(/Microsoft.Fixture/, "agiru::Fixture"); changed++
      }
      control == "lost-multiple-new-lines" && /\.multipleNewLines = true/ { changed++; next }
      { print }
      END { if (changed != 1) exit 2 }
    ' "$source" > "$proof/$control.cpp"
    mv "$proof/$control.cpp" "$source"
  done
  mapfile -t control_sources < <(rg --files --no-ignore "$proof/$control" -g '*.cpp' | LC_ALL=C sort)
  [ "${#control_sources[@]}" -eq "${#sources[@]}" ]
  "$CXX" "${flags[@]}" "-I$proof/$control/fixture" "-I$proof/$control/absent" \
    "-I$proof/$control/shared" "$proof/runner.o" "${control_sources[@]}" "${links[@]}" \
    -o "$proof/$control-runner" > "$proof/$control.compile.log" 2>&1
  if "$proof/$control-runner" > "$proof/$control-execution.log" 2>&1; then
    printf 'control-extensions: %s escaped page metadata execution\n' "$control" >&2
    exit 1
  fi
  case "$control" in
    missing-page-owner) claim='an extended page retains its original application';;
    cpp-page-namespace) claim='page namespace remains its original AL spelling';;
    lost-multiple-new-lines) claim='page/request-page MultipleNewLines survives extension composition';;
  esac
  rg -q "FAIL .*${claim}" "$proof/$control-execution.log"
  rm -- "$proof/$control-runner"
  rm -r -- "$proof/$control"
done

for kind in Page Report; do
  original="$proof/extension/Controls.$kind"Ext.al
  cp "$original" "$proof/original-$kind.al"
  for variant in missing cycle; do
    if [ "$variant" = missing ]; then
      sed 's/addafter(Later)/addafter(Missing)/' "$proof/original-$kind.al" > "$original"
    else
      sed 's/addlast(Fields)/addafter(Dependent)/' "$proof/original-$kind.al" > "$original"
    fi
    if cmp -s "$original" "$proof/original-$kind.al"; then
      printf 'control-extensions: %s %s control did not match\n' "$kind" "$variant" >&2
      exit 1
    fi
    for mode in generated analysis; do
      output=()
      if [ "$mode" = generated ]; then
        target="$proof/$kind-$variant-output"
        cp -a "$proof/generated" "$target"
        output=("$target")
        printf '%s\n' 'preserve incomplete output' > "$target/Previous.h"
      fi
      if "$B/agirutc" "$proof" "$proof/apps.json" "${output[@]}" \
        > "$proof/$kind-$variant-$mode.log" 2>&1; then
        printf 'control-extensions: %s %s %s escaped refusal\n' "$kind" "$variant" "$mode" >&2
        exit 1
      fi
      rg -q '^ABORT +[1-9][0-9]* extension control operation' "$proof/$kind-$variant-$mode.log"
      if [ "$mode" = generated ]; then rg -q '^preserve incomplete output$' "$target/Previous.h"; fi
    done
  done
  cp "$proof/original-$kind.al" "$original"
done
original="$proof/extension/Controls.PageExt.al"
sed 's/addafter(Original)/addbefore(Original)/' "$proof/original-Page.al" > "$original"
if cmp -s "$original" "$proof/original-Page.al"; then
  printf 'control-extensions: order control did not match\n' >&2
  exit 1
fi
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/wrong-order" > "$proof/wrong-order.log" 2>&1
mapfile -t wrong_sources < <(rg --files --no-ignore "$proof/wrong-order" -g '*.cpp' | LC_ALL=C sort)
[ "${#wrong_sources[@]}" -gt 0 ]
"$CXX" "${flags[@]}" "-I$proof/wrong-order/fixture" "-I$proof/wrong-order/absent" \
  "-I$proof/wrong-order/shared" "$proof/runner.o" "${wrong_sources[@]}" "${links[@]}" \
  -o "$proof/wrong-order-runner"
if "$proof/wrong-order-runner" > "$proof/wrong-order-execution.log" 2>&1; then
  printf 'control-extensions: wrong sibling order escaped execution\n' >&2
  exit 1
fi
rg -q 'extension operations preserve declared sibling order' "$proof/wrong-order-execution.log"
cp "$proof/original-Page.al" "$original"
rm -f "$proof/runner.o" "$proof/runner" "$proof/wrong-order-runner"
printf 'control-extensions: page/request-page order, properties and source identity execute; missing/cyclic anchors refuse; three compiled metadata controls reject; %s\n' "$proof"
