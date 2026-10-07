#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-xmlport-import.XXXXXX)
trap 'find "$proof" -type f \( -name "*.o" -o -name "runner" -o -name "control" -o -name "mutant.cpp" \) -delete' EXIT
printf 'xmlport-import: receipts %s\n' "$proof"
sha256sum test/runtime/xmlport-import.sh test/runtime/xmlport-import/* \
  src/gen/{BodyWriter,PageWriter}.cpp "$B/agirutc" "$B/libagiru_gen.so" "$B/libagiru_al.so" \
  "$B/libagiru_db.so" "$B/libagiru_net.so" "$B/libagiru_rt.so" > "$proof/inputs.sha256"
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
for profile in default-false default-true omitted temporary; do
  case_root="$proof/$profile"
  mkdir -p "$case_root/source"
  cp test/runtime/xmlport-import/Fixture.Table.al "$case_root/source/Fixture.Table.al"
  cp test/transpiler/native-enums/source/app.json "$case_root/source/app.json"
  defaults=true
  temporary=false
  if [ "$profile" = default-false ]; then defaults=false; fi
  if [ "$profile" = temporary ]; then temporary=true; fi
  awk -v defaults="$defaults" -v temporary="$temporary" -v profile="$profile" '
    /DefaultFieldsValidation = false;/ {
      changed++;
      if (profile == "omitted") next;
      sub(/false/, defaults)
    }
    /UseTemporary = false;/ { sub(/false/, temporary); changed++ }
    profile == "omitted" && /FieldValidate = Undefined;/ { changed++; next }
    { print }
    END { if (changed != (profile == "omitted" ? 3 : 2)) exit 2 }
  ' test/runtime/xmlport-import/Fixture.XmlPort.al > "$case_root/source/Fixture.XmlPort.al"
  printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$case_root/apps.json"
  printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$case_root/scope.json"
  "$B/agirutc" "$case_root" "$case_root/apps.json" "$case_root/generated" \
    > "$case_root/generation.log" 2>&1
  flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
    "-I$case_root/generated/fixture" "-I$case_root/generated/absent" "-I$case_root/generated/shared")
  mapfile -t sources < <(rg --files --no-ignore "$case_root/generated" -g '*.cpp' | LC_ALL=C sort)
  [ "${#sources[@]}" -eq 4 ]
  port="$case_root/generated/fixture/fixture/xmlport/ImportValidationConsumer.cpp"
  objects=()
  for source in "${sources[@]}"; do
    if [ "$source" = "$port" ]; then continue; fi
    object="$case_root/${source##*/}.o"
    "$CXX" "${flags[@]}" -c "$source" -o "$object"
    objects+=("$object")
  done
  "$CXX" "${flags[@]}" -c test/runtime/xmlport-import/Runner.cpp -o "$case_root/runner.o"
  "$CXX" "${flags[@]}" "$case_root/runner.o" "${objects[@]}" "$port" \
    "${links[@]}" -o "$case_root/runner"
  "$case_root/runner" "$dsn" "$temporary" "$defaults" | tee "$case_root/execution.log"
  mkdir -p "$B/fixture-commands"
  jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/xmlport-import/Runner.cpp" \
    --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
    "$CXX" "${flags[@]}" -c test/runtime/xmlport-import/Runner.cpp -o "$case_root/runner.o" \
    > "$B/fixture-commands/xmlport-import.json"
  controls=()
  case "$profile" in
    default-false) controls=(ignore-default) ;;
    default-true) controls=(early-validation undefined-no swallowed-error) ;;
    temporary) controls=(validate-temporary) ;;
  esac
  for control in "${controls[@]}"; do
    awk -v control="$control" '
      control == "early-validation" && /OnAfterAssignFieldValue\(\);/ {
        print "    Row->Validate(Row->Value);"; changed++
      }
      control == "early-validation" && /Row->Validate\(Row->Value\);/ { next }
      control == "swallowed-error" && /Row->Validate\(Row->Value\);/ {
        print "    try { Row->Validate(Row->Value); } catch (const ::agiru::Error &) {}";
        changed++; next
      }
      control == "undefined-no" && /Row->Validate\(Row->Other\);/ { changed++; next }
      { print }
      control == "ignore-default" && /OnAfterAssignFieldOther\(\);/ {
        print "    Row->Validate(Row->Other);"; changed++
      }
      control == "validate-temporary" && /OnAfterAssignFieldValue\(\);/ {
        print "    Row->Validate(Row->Value);"; changed++
      }
      END { if (changed != 1) exit 2 }
    ' "$port" > "$case_root/mutant.cpp"
    "$CXX" "${flags[@]}" "-I$(dirname "$port")" "$case_root/runner.o" "${objects[@]}" \
      "$case_root/mutant.cpp" "${links[@]}" -o "$case_root/control"
    status=0
    "$case_root/control" "$dsn" "$temporary" "$defaults" \
      > "$case_root/$control.log" 2>&1 || status=$?
    [ "$status" -eq 1 ]
    case "$control" in
      early-validation) expected='explicit Yes validates the changed element' ;;
      undefined-no|ignore-default) expected='Undefined inherits the XMLport default' ;;
      swallowed-error) expected='physical field validation errors propagate' ;;
      validate-temporary) expected='temporary XMLport sources bypass field validation' ;;
    esac
    rg -q "$expected" "$case_root/$control.log"
    sha256sum "$case_root/mutant.cpp" "$case_root/control" >> "$proof/controls.sha256"
    rm -- "$case_root/mutant.cpp" "$case_root/control"
  done
  if [ "$profile" = default-false ]; then
    header="$case_root/generated/fixture/fixture/xmlport/ImportValidationConsumer.h"
    cp "$header" "$case_root/original.h"
    awk '
      /ImportValidationConsumer_XmlPort\(\) = default;/ { changed++; next }
      { print }
      END { if (changed != 1) exit 2 }
    ' "$case_root/original.h" > "$header"
    status=0
    "$CXX" "${flags[@]}" -c test/runtime/xmlport-import/Runner.cpp \
      -o "$case_root/control.o" > "$case_root/aggregate-construction.log" 2>&1 || status=$?
    [ "$status" -eq 1 ]
    rg -q 'private default constructor' "$case_root/aggregate-construction.log"
    cp "$case_root/original.h" "$header"
    rm -- "$case_root/original.h"
  fi
done
sha256sum --check "$proof/inputs.sha256" > "$proof/input-integrity.log"
printf 'xmlport-import: generated field/attribute assignment, validation/defaults, temporary import and explicit Validate pass; five execution controls and aggregate-construction defect reject; %s\n' "$proof"
