#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-page-profile.XXXXXX)
sha256sum include/runtime/{PageCore,PageDispatcher,PageSession,PageHtml,PageValue}.h \
  src/rt/{PageCore,PageDispatcher,PageHtml,PageValue}.cpp \
  include/type/Utf8.h src/net/Encoding.cpp \
  test/gate/{PageHtmlGate,PageValueGate}.cpp test/ui/page-profile.sh > "$proof/inputs.sha256"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
for gate in PageHtmlGate PageValueGate; do
  "$B/gate_$gate" > "$proof/$gate.log" 2>&1
  cat "$proof/$gate.log"
done
for control in no-escape display-as-value no-disabled no-byte-budget no-scale no-closing display-enum no-flowfilter-refusal no-presentation-auth; do
  source=src/rt/PageHtml.cpp
  gate=PageHtmlGate
  case "$control" in no-scale|no-closing|display-enum|no-flowfilter-refusal) source=src/rt/PageValue.cpp; gate=PageValueGate;; esac
  awk -v control="$control" '
    control == "no-escape" && /Raw\("&lt;"\)/ {
      sub(/Raw\("&lt;"\)/, "Raw(\"<\")"); changed++
    }
    control == "display-as-value" && /out.Attribute\("data-value", value.value\)/ {
      sub(/out.Attribute\("data-value", value.value\)/, "out.Attribute(\"data-value\", \"1.23\")"); changed++
    }
    control == "no-disabled" && /if \(!action.enabled\)/ {
      sub(/!action.enabled/, "false"); changed++
    }
    control == "no-byte-budget" && /if \(text.size\(\) > limits_.bytes - output_.size\(\)\)/ {
      sub(/text.size\(\) > limits_.bytes - output_.size\(\)/, "false"); changed++
    }
    control == "no-scale" && /default: result.value = FieldText\(record, field\); break;/ {
      sub(/result.value = FieldText\(record, field\)/, "result.value = field.type == FieldType::Decimal ? \"1.23\" : FieldText(record, field)"); changed++
    }
    control == "no-closing" && /result.closing = At<Date>\(record, field\).IsClosing\(\)/ {
      sub(/At<Date>\(record, field\).IsClosing\(\)/, "false"); changed++
    }
    control == "display-enum" && /result.value = std::to_string\(ordinal\)/ {
      sub(/std::to_string\(ordinal\)/, "FieldText(record, field)"); changed++
    }
    control == "no-flowfilter-refusal" && /if \(field.fieldClass == FieldClass::FlowFilter\)/ {
      sub(/field.fieldClass == FieldClass::FlowFilter/, "false"); changed++
    }
    control == "no-presentation-auth" && /authorization.Require\(declaration.id, presentation\)/ {
      sub(/authorization.Require\(declaration.id, presentation\)/, "static_cast<void>(presentation)"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "test/gate/$gate.cpp" "$proof/$control.cpp" \
    "${links[@]}" -o "$proof/$control"
  status=0
  "$proof/$control" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  rg -q '^FAIL  ' "$proof/$control.log"
  rm -- "$proof/$control" "$proof/$control.cpp"
done
printf 'page-profile: nine scalar/HTML execution mutants rejected; not HTTP/CMD/MCP parity\n'
printf 'page-profile: %s\n' "$proof"
