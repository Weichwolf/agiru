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
for control in cursor-local close-local raw-load unfiltered-policy ignore-policy prohibit-boundary; do
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
sha256sum src/net/XmlReader.cpp include/dotnet/XmlReader.h test/gate/XmlReaderGate.cpp \
  "$B/libagiru_net.so" "$gate" > "$proof/inputs.sha256"
printf 'xml-reader: shared cursor/close/Load and DTD checks pass; six source controls and resource trap reject; %s\n' "$proof"
