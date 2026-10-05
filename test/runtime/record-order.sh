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
lifecycle="$B/gate_CursorLifecycleGate"
"$lifecycle" > "$proof/lifecycle.log" 2>&1
dynamic="$B/gate_DynamicRecordGate"
"$dynamic" > "$proof/dynamic.log" 2>&1
rename="$B/gate_RenameGate"
"$rename" > "$proof/rename.log" 2>&1
fetch_block=$(sed -n 's/^inline constexpr std::size_t kFetchBlock = \([0-9]*\);$/\1/p' src/rt/Cursor.h)
[[ "$fetch_block" =~ ^[1-9][0-9]*$ ]]
bounded_walks() {
  awk -v block="$fetch_block" '
    /^cursor-walk: begin / {
      if (active) refused = 1;
      active = 1; calls = 0; steps = $3 < 0 ? -$3 : $3;
      limit = 5 + int((steps + block - 1) / block); next
    }
    /^sql: / && active { calls++ }
    /^cursor-walk: end / {
      if (!active || calls > limit) refused = 1;
      printf "cursor-walk: %d steps, %d SQL statements, bound %d\n", steps, calls, limit;
      active = 0; phases++
    }
    END { if (active || phases != 24 || refused) exit 1 }
  ' "$1"
}
AGIRU_TRACE_SQL=1 "$lifecycle" --trace-walks > "$proof/lifecycle-trace.log" 2>&1
bounded_walks "$proof/lifecycle-trace.log" > "$proof/lifecycle-statements.log"
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
for control in cursor-generation surviving-portal released-portal one-row-fetch; do
  source=src/rt/Cursor.cpp
  if [ "$control" = cursor-generation ]; then source=src/rt/Navigate.cpp; fi
  awk -v control="$control" '
    control == "cursor-generation" && /if \(open != nullptr && .*open->cursor.Current\(\)/ {
      sub(/!open->cursor.Current\(\)/, "false"); changed++
    }
    control == "surviving-portal" && /if \(Session::Current\(\).Transaction\(\).CursorEpoch\(\) != epoch_\)/ {
      print "    if (Session::Current().Transaction().CursorEpoch() != epoch_) { return; }";
      skip = 1; changed++; next
    }
    skip { if (/^    }$/) skip = 0; next }
    control == "released-portal" && /if \(!connection_->InTransaction\(\) \|\| connection_->InFailedTransaction\(\)\)/ {
      print "  if (Session::Current().Transaction().Depth() == 0) { return; }"; changed++
    }
    control == "one-row-fetch" && /std::to_string\(kFetchBlock\)/ {
      sub(/std::to_string\(kFetchBlock\)/, "std::to_string(std::size_t{1})"); changed++
    }
    { print }
    END { if (changed != 1 || skip) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_rt -lagiru_db -lagiru_net -o "$proof/$control.so"
  if [ "$control" = one-row-fetch ]; then
    AGIRU_TRACE_SQL=1 LD_PRELOAD="$proof/$control.so" "$lifecycle" --trace-walks > "$proof/$control.log" 2>&1
    if bounded_walks "$proof/$control.log" > "$proof/$control-statements.log"; then
      printf 'record-order: per-row SQL escaped the bounded-walk control\n' >&2
      exit 1
    fi
  else
    if LD_PRELOAD="$proof/$control.so" "$lifecycle" > "$proof/$control.log" 2>&1; then
      printf 'record-order: %s escaped the cursor-lifecycle gate\n' "$control" >&2
      exit 1
    fi
    rg -q 'FAIL ' "$proof/$control.log"
  fi
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
for control in write-revision insert-notify update-notify delete-notify bulk-delete-notify table-scope connection-scope reader-retirement; do
  source=src/rt/RecordChanges.cpp
  case "$control" in
    insert-notify|update-notify|delete-notify) source=src/rt/Storage.cpp;;
    bulk-delete-notify) source=src/rt/Navigate.cpp;;
  esac
  awk -v control="$control" '
    /^std::optional<FieldValues> InsertRow/ { inserting = 1 }
    /^std::optional<FieldValues> Updated/ { updating = 1 }
    /^}/ { inserting = 0; updating = 0 }
    control == "write-revision" && /return observed_ == revision_->second.value;/ {
      sub(/observed_ == revision_->second.value/, "true"); changed++
    }
    control == "insert-notify" && inserting && /^  detail::RecordWritten/ { changed++; next }
    control == "update-notify" && updating && /^  detail::RecordWritten/ { changed++; next }
    control == "delete-notify" && /if \(deleted\) \{ detail::RecordWritten/ { changed++; next }
    control == "bulk-delete-notify" && /if \(written.Affected\(\) != 0\) \{ RecordWritten/ { changed++; next }
    control == "table-scope" && /if \(found != state->recordChanges->tables_.end\(\)\)/ {
      print "  for (auto &[id, revision] : state->recordChanges->tables_) { ++revision.value; }";
      print "  static_cast<void>(found);"; changed++; next
    }
    control == "connection-scope" && /&Session::Current\(\).Database\(\) != &connection/ {
      sub(/&Session::Current\(\).Database\(\) != &connection/, "false"); changed++
    }
    control == "connection-scope" && /^void RecordWritten/ {
      print; print "  static_cast<void>(connection);"; next
    }
    control == "reader-retirement" && /owner_->tables_.erase\(revision_\)/ {
      sub(/owner_->tables_.erase\(revision_\)/, "static_cast<void>(revision_)"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "$source" > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_rt -lagiru_db -lagiru_net -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$dynamic" > "$proof/$control.log" 2>&1; then
    printf 'record-order: %s escaped the dynamic-record gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
control=modifyall-frame
mkdir -p "$proof/$control/runtime"
awk '
  /void ModifyAll\(Field &member, const Value &value, Boolean RunTrigger = false\)/ {
    print;
    print "    if (!FindSet()) { return; }";
    print "    do { member = value; Modify(RunTrigger); } while (Next() != 0);";
    skip = 1; changed++; next
  }
  skip { if (/^  }$/) { print; skip = 0; } next }
  { print }
  END { if (changed != 1 || skip) exit 2 }
' include/runtime/Table.h > "$proof/$control/runtime/Table.h"
dsn=$(sed -n 's/^AGIRU_TEST_DSN:STRING=//p' "$B/CMakeCache.txt")
[[ -n "$dsn" ]]
image="$B/CMakeFiles/gate_image.dir/test/transpiler/golden"
"$CXX" -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
  --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
  "-DAGIRU_TEST_DSN=\"$dsn\"" -I"$proof/$control" -Iinclude -Isrc/rt \
  -Itest/transpiler/golden test/gate/DynamicRecordGate.cpp \
  "$image/LineNumberBuffer.cpp.o" "$image/ResourceCost.cpp.o" \
  "$image/ResourceCost.def.cpp.o" "$image/TransferOldExtTextLines.cpp.o" \
  "$image/WorkType.cpp.o" "$image/WorkType.def.cpp.o" \
  -L"$B" -Wl,-rpath,"$B" -lagiru_gen -lagiru_rt -lagiru_al -lagiru_net -lagiru_db \
  -o "$proof/$control-gate"
if "$proof/$control-gate" > "$proof/$control.log" 2>&1; then
  printf 'record-order: caller traversal escaped the ModifyAll frame gate\n' >&2
  exit 1
fi
rg -q 'FAIL .*ModifyAll retains the caller.s field buffer' "$proof/$control.log"
sha256sum "$proof/$control/runtime/Table.h" "$proof/$control-gate" >> "$proof/controls.sha256"
rm -- "$proof/$control/runtime/Table.h" "$proof/$control-gate"
rmdir "$proof/$control/runtime" "$proof/$control"
control=cascade-anchor
awk '
  /} while \(RuntimeNext\(row.record, referring, 1\) != 0\);/ {
    print "    reference.entry->copy(row.record, written.record);"; changed++
  }
  { print }
  END { if (changed != 1) exit 2 }
' src/rt/Rename.cpp > "$proof/$control.cpp"
"$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
  -lagiru_rt -lagiru_db -lagiru_net -o "$proof/$control.so"
if LD_PRELOAD="$proof/$control.so" "$rename" > "$proof/$control.log" 2>&1; then
  printf 'record-order: changed cascade anchor escaped the rename gate\n' >&2
  exit 1
fi
rg -q 'FAIL .*every referring row follows the renamed key' "$proof/$control.log"
sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
rm -- "$proof/$control.cpp" "$proof/$control.so"
sha256sum src/rt/RecordOrder.h src/rt/RecordOrder.cpp src/rt/Navigate.cpp \
  src/rt/Cursor.h src/rt/Cursor.cpp \
  src/rt/RecordChanges.h src/rt/RecordChanges.cpp src/rt/SessionState.h src/rt/Storage.cpp \
  src/rt/Selection.cpp src/rt/Temporary.cpp src/rt/RecordState.cpp \
  include/runtime/RecordState.h include/runtime/Table.h \
  test/gate/MixedOrderGate.cpp test/gate/SelectionChangeGate.cpp test/gate/CursorLifecycleGate.cpp \
  test/gate/DynamicRecordGate.cpp src/rt/Rename.cpp test/gate/RenameGate.cpp \
  "$B/libagiru_rt.so" "$gate" "$selection" "$lifecycle" "$dynamic" "$rename" > "$proof/inputs.sha256"
printf 'record-order: order, selections, cursor lifecycle, dynamic writes and rename cascades pass; twenty-three controls reject; %s\n' "$proof"
