#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-record-order-controls.XXXXXX)
gate="$B/gate_MixedOrderGate"
"$gate" > "$proof/current.log" 2>&1
selection="$B/gate_SelectionChangeGate"
"$selection" > "$proof/selection.log" 2>&1
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude -Isrc/rt --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
for control in field-direction global-direction primary-ties uniform-predicate; do
  source=src/rt/RecordOrder.cpp
  if [ "$control" = uniform-predicate ]; then source=src/rt/Navigate.cpp; fi
  awk -v control="$control" '
    control == "field-direction" && /\.ascending = ascending == direction/ {
      sub(/ascending == direction/, "ascending == (direction || true)"); changed++
    }
    control == "global-direction" && /\.ascending = ascending == direction/ {
      sub(/ascending == direction/, "(ascending || true) == direction"); changed++
    }
    control == "primary-ties" && /if \(table.keys.empty\(\)\) \{ return; \}/ {
      sub(/table.keys.empty\(\)/, "table.keys.empty() || !key.empty()"); changed++
    }
    control == "uniform-predicate" && /const bool uniform = std::ranges::all_of/ {
      print "  const bool uniform = true;"; skip = 1; changed++; next
    }
    skip { if (/\}\);/) skip = 0; next }
    { print }
    END { if (changed != 1 || skip) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_rt -lagiru_db -lagiru_net -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1; then
    printf 'record-order: %s escaped the mixed-order gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
for control in selection-cache temporary-cache excluded-anchor mutation-version noop-cursor; do
  source=src/rt/Temporary.cpp
  case "$control" in selection-cache|noop-cursor) source=src/rt/RecordState.cpp;; esac
  awk -v control="$control" '
    control == "selection-cache" && /^void SelectionChanged\(RecordState &state\)/ {
      print "void SelectionChanged(RecordState &state) { static_cast<void>(state); }";
      skip = 1; changed++; next
    }
    skip { if (/^}/) skip = 0; next }
    control == "temporary-cache" && /const bool selectionChanged = held.state->viewDirty;/ {
      sub(/held.state->viewDirty/, "false"); changed++
    }
    control == "excluded-anchor" && /if \(wanted > 0 && anchor.found\)/ {
      sub(/wanted > 0 && anchor.found/, "wanted > 0"); changed++
    }
    /^bool TempModify/ { modifying = 1 }
    modifying && /^}/ { modifying = 0 }
    control == "mutation-version" && modifying && /\+\+held.temp->version;/ {
      changed++; next
    }
    control == "noop-cursor" && /previous == state.filters.end\(\) \? text.empty\(\) : previous->text == text/ {
      sub(/return;/, "SelectionChanged(state); return;"); changed++
    }
    { print }
    END { if (changed != 1 || skip) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_rt -lagiru_db -lagiru_net -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$selection" > "$proof/$control.log" 2>&1; then
    printf 'record-order: %s escaped the selection-change gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
sha256sum src/rt/RecordOrder.h src/rt/RecordOrder.cpp src/rt/Navigate.cpp \
  src/rt/Selection.cpp src/rt/Temporary.cpp src/rt/RecordState.cpp \
  include/runtime/RecordState.h include/runtime/Table.h \
  test/gate/MixedOrderGate.cpp test/gate/SelectionChangeGate.cpp \
  "$B/libagiru_rt.so" "$gate" "$selection" > "$proof/inputs.sha256"
printf 'record-order: mixed order and changing selections pass; nine controls reject; %s\n' "$proof"
