#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-xml-reader.XXXXXX)
gate="$B/gate_XmlReaderGate"
"$gate" > "$proof/shared-state.log" 2>&1
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude -Isrc/net --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
for control in cursor-local close-local raw-load; do
  awk -v control="$control" '
    { print }
    (control == "cursor-local" && $0 == "Boolean XmlReader::Read() {") ||
    (control == "close-local" && $0 == "void XmlReader::Close() {") {
      print "  if (state_ != nullptr) { state_ = std::make_shared<State>(*state_); }"; changed++
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
  "$CXX" "${flags[@]}" "$proof/$control.cpp" $(pkg-config --cflags --libs libxml-2.0) \
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
  else
    rg -q 'positioned Load starts at the current child' "$proof/$control.log"
    rg -q 'closed reader Load replaces the old tree with an empty document' "$proof/$control.log"
    rg -q 'Load advances all reader aliases to EOF' "$proof/$control.log"
  fi
  rm -- "$proof/$control.so" "$proof/$control.cpp"
done
printf 'xml-reader: shared cursor/close/Load checks pass; three source controls reject; %s\n' "$proof"
