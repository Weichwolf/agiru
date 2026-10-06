#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-xml-reader.XXXXXX)
gate="$B/gate_XmlReaderGate"
"$gate" > "$proof/shared-state.log" 2>&1
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -Iinclude -Isrc/net)
link_flags=(-shared --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
resource_source=test/runtime/fixtures/XmlReaderResourceTrap.cpp
read -r -a xml_flags <<< "$(pkg-config --cflags-only-I libxml-2.0)"
system_includes=()
for flag in "${xml_flags[@]}"; do system_includes+=(-isystem "${flag#-I}"); done
resource_command=("$CXX" "${flags[@]}" "${system_includes[@]}" -c "$resource_source"
  -o "$proof/resource-trap.o")
"${resource_command[@]}"
"$CXX" "${flags[@]}" "${link_flags[@]}" "$proof/resource-trap.o" \
  $(pkg-config --libs libxml-2.0) -o "$proof/resource-trap.so"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/$resource_source" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "${resource_command[@]}" > "$B/fixture-commands/xml-reader-resource.json"
LD_PRELOAD="$proof/resource-trap.so" "$gate" > "$proof/resource-trap.log" 2>&1
dom="$B/gate_XmlGate"
"$dom" > "$proof/dom.log" 2>&1
LD_PRELOAD="$proof/resource-trap.so" "$dom" > "$proof/dom-context-trap.log" 2>&1
for control in root-classification first-error utf16-column global-context; do
  awk -v control="$control" '
    control == "root-classification" && /const bool rootText =/ {
      sub(/byte_ !=/, "byte_ =="); changed++
    }
    control == "first-error" && /failure->error_.code != XML_ERR_OK/ {
      sub(/failure->error_.code != XML_ERR_OK/, "false"); changed++
    }
    control == "utf16-column" && /return scalarColumn \+ extra;/ {
      sub(/\+ extra/, "- extra"); changed++
    }
    { print }
    control == "global-context" && /context->sax->serror = &XmlParseFailure::Capture;/ {
      print "  ::xmlSetStructuredErrorFunc(context.get(), &XmlParseFailure::Capture);"; changed++
    }
    END { if (changed != 1) exit 2 }
  ' src/net/XmlEngine.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "${link_flags[@]}" "$proof/$control.cpp" \
    $(pkg-config --cflags --libs libxml-2.0) "-L$B" "-Wl,-rpath,$B" \
    -lagiru_net -o "$proof/$control.so"
  status=0
  if [ "$control" = global-context ]; then
    LD_PRELOAD="$proof/resource-trap.so:$proof/$control.so" "$dom" \
      > "$proof/$control.log" 2>&1 || status=$?
    [ "$status" -eq 134 ]
    rg -q 'fixture global-error-handler-write' "$proof/$control.log"
  else
    LD_PRELOAD="$proof/$control.so" "$dom" > "$proof/$control.log" 2>&1 || status=$?
    [ "$status" -eq 1 ]
  fi
  if [ "$control" = first-error ]; then
    rg -q 'first parse failure is not replaced' "$proof/$control.log"
  elif [ "$control" != global-context ]; then
    rg -q 'XML failures keep their own qualified classification and location' "$proof/$control.log"
  fi
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/dom-controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
for control in cursor-local close-local raw-load unfiltered-policy ignore-policy prohibit-boundary doctype-header; do
  awk -v control="$control" '
    { print }
    (control == "cursor-local" && $0 == "Boolean XmlReader::Read() {") ||
    (control == "close-local" && $0 == "void XmlReader::Close() {") {
      print "  if (state_ != nullptr) { state_ = std::make_shared<State>(*state_); }"; changed++
    }
    control == "unfiltered-policy" && $0 == "std::string ApplyDtdPolicy(std::string &text, const XmlReaderSettings &settings) {" {
      print "  static_cast<void>(text); static_cast<void>(settings); return {};"; changed++
    }
    control == "ignore-policy" && $0 == "  const auto processing = settings.DtdProcessing().Number();" {
      print "  if (processing == 1) { return {}; }"; changed++
    }
    control == "prohibit-boundary" && $0 == "Boolean XmlReader::Read() {" {
      print "  if (state_ != nullptr) { state_->policyFailure.clear(); }"; changed++
    }
    control == "doctype-header" && $0 == "std::size_t DoctypeHeaderEnd(const MarkupBytes &source, std::size_t start, std::size_t size) {" {
      print "  while (start < size && source.At(start) != '\''>'\'') { start += source.Width(); }";
      print "  return start;"; changed++
    }
    control == "raw-load" && $0 == "void XmlReader::LoadDocument(::agiru::detail::XmlHandle &into, bool preserveWhitespace) const {" {
      print "  if (state_ != nullptr && state_->text != nullptr) {";
      print "    if (!::agiru::detail::Parse(*state_->text, preserveWhitespace, into)) {";
      print "      throw Error(\"XmlDocument.Load: malformed raw XML\");";
      print "    }";
      print "    return;";
      print "  }"; changed++
    }
    END { if (changed != 1) exit 2 }
  ' src/net/XmlReader.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "${link_flags[@]}" "$proof/$control.cpp" $(pkg-config --cflags --libs libxml-2.0) \
    "-L$B" "-Wl,-rpath,$B" -lagiru_net -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1; then
    printf 'xml-reader: %s escaped the shared-state gate\n' "$control" >&2
    exit 1
  fi
  if [ "$control" = cursor-local ]; then
    rg -q 'the other alias sees the declaration' "$proof/$control.log"
    rg -q 'EOF belongs to the shared cursor' "$proof/$control.log"
  elif [ "$control" = close-local ]; then
    rg -q 'closing an alias ends the shared walk' "$proof/$control.log"
  elif [ "$control" = raw-load ]; then
    rg -q 'positioned Load starts at the current child' "$proof/$control.log"
    rg -q 'closed reader Load replaces the old tree with an empty document' "$proof/$control.log"
    rg -q 'Load advances all reader aliases to EOF' "$proof/$control.log"
  elif [ "$control" = ignore-policy ]; then
    rg -q 'Ignore never reports a discarded doctype node' "$proof/$control.log"
    rg -q 'Ignore leaves DTD-defined entity references undeclared' "$proof/$control.log"
  elif [ "$control" = doctype-header ]; then
    rg -q 'Ignore rejects malformed DOCTYPE header or closing syntax' "$proof/$control.log"
  else
    rg -q 'default Prohibit rejects a DTD before reader or DOM expansion' "$proof/$control.log"
  fi
  if [ "$control" = unfiltered-policy ]; then
    if LD_PRELOAD="$proof/resource-trap.so:$proof/$control.so" "$gate" \
        > "$proof/unfiltered-resource-trap.log" 2>&1; then
      printf 'xml-reader: unfiltered input escaped the external-resource trap\n' >&2
      exit 1
    fi
    rg -q 'fixture external-resource-request' "$proof/unfiltered-resource-trap.log"
  fi
  rm -- "$proof/$control.so" "$proof/$control.cpp"
done
rm -- "$proof/resource-trap.so" "$proof/resource-trap.o"
sha256sum src/net/{XmlReader,XmlEngine,DotNetXml}.cpp src/net/XmlEngine.h \
  include/dotnet/XmlReader.h test/gate/{XmlReader,Xml}Gate.cpp "$resource_source" \
  "$B/libagiru_net.so" "$gate" "$dom" > "$proof/inputs.sha256"
printf 'xml-reader: DOM diagnostics, shared cursor/close/Load and DTD checks pass; eleven source controls and resource/context traps reject; %s\n' "$proof"
