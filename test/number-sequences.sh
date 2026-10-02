#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d "$B/number-sequences.XXXXXX")
gate="$B/gate_NumberSequenceGate"
restore() {
  local status=$?
  trap - EXIT
  "$gate" --process-cleanup > "$proof/range-cleanup.log" 2>&1 || status=1
  "$gate" --names-cleanup > "$proof/name-cleanup.log" 2>&1 || status=1
  "$gate" --roundtrips > "$proof/restored.log" 2>&1 || status=1
  exit "$status"
}
trap restore EXIT

"$gate" --process-setup > "$proof/process-setup.log" 2>&1
pids=()
for worker in 1 2 3; do
  "$gate" --process-worker > "$proof/process-$worker.log" 2>&1 &
  pids+=("$!")
done
red=0
for pid in "${pids[@]}"; do
  if ! wait "$pid"; then red=$((red + 1)); fi
done
"$gate" --process-cleanup > "$proof/process-cleanup.log" 2>&1
[ "$red" -eq 0 ]
rg --no-filename '^RESERVATION ' "$proof"/process-[123].log |
  LC_ALL=C sort -n -k2 |
  awk '$2 != expected { exit 1 } { expected += 300; count++ } END { if (count != 300) exit 1 }'

AGIRU_TRACE_SQL=1 "$gate" --roundtrips > "$proof/roundtrips.log" 2>&1
[ "$(rg -c '^sql: SELECT value, increment, present.* \| reserve \|' "$proof/roundtrips.log")" = 2 ]
rg -q ' \| 1000000000$' "$proof/roundtrips.log"

flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -fPIC -shared -Iinclude
  -stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_db)

awk '
  { print }
  /INTO STRICT previous, called;/ { print "  PERFORM pg_catalog.pg_sleep(0.02);"; matches++ }
  END { if (matches != 1) exit 2 }
' src/rt/NumberSequenceStorage.cpp > "$proof/Paused.cpp"
"$CXX" "$proof/Paused.cpp" "${flags[@]}" -o "$proof/paused.so"
LD_PRELOAD="$proof/paused.so" "$gate" --concurrency > "$proof/serialized-paused.log" 2>&1
LD_PRELOAD="$proof/paused.so" "$gate" --cancellation > "$proof/cancellation.log" 2>&1

awk '
  /pg_advisory_lock\(identity_key \| 1\)/ { locks++; next }
  /pg_advisory_unlock\(identity_key \| 1\)/ { unlocks++; next }
  { print }
  END { if (locks != 1 || unlocks != 3) exit 2 }
' "$proof/Paused.cpp" > "$proof/Unlocked.cpp"
"$CXX" "$proof/Unlocked.cpp" "${flags[@]}" -o "$proof/unlocked.so"
if LD_PRELOAD="$proof/unlocked.so" "$gate" --concurrency > "$proof/unlocked.log" 2>&1; then
  printf 'number-sequences: removing allocation serialization escaped the gate\n' >&2
  exit 1
fi
rg -q 'two Range sessions and Next never reserve overlapping values' "$proof/unlocked.log"

awk '
  /\.count = Count\}\);/ {
    sub(/\.count = Count/, ".count = 1"); print
    print "  for (Integer taken = 1; taken < Count; ++taken) {"
    print "    const Result discarded = Operate({.operation = \"reserve\", .name = Name, .companySpecific = CompanySpecific});"
    print "  }"
    matches++; next
  }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/NumberSequence.cpp > "$proof/PerValue.cpp"
"$CXX" "$proof/PerValue.cpp" "${flags[@]}" -o "$proof/per-value.so"
if LD_PRELOAD="$proof/per-value.so" "$gate" --concurrency > "$proof/per-value.log" 2>&1; then
  printf 'number-sequences: the former per-value loop escaped the gate\n' >&2
  exit 1
fi
rg -q 'two Range sessions and Next never reserve overlapping values' "$proof/per-value.log"
awk '
  BEGIN { apostrophe = sprintf("%c", 39) }
  /\$1::text, \$2::text, \$3::boolean/ {
    print "      \"$1::text, " apostrophe "\" + std::string(request.name) + \"" apostrophe "::text, $3::boolean, $4::text, $5::bigint, $6::bigint, $7::integer) WHERE $2::text IS NOT NULL\","
    matches++; next
  }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/NumberSequence.cpp > "$proof/UnboundName.cpp"
"$CXX" "$proof/UnboundName.cpp" "${flags[@]}" -o "$proof/unbound-name.so"
LD_PRELOAD="$proof/unbound-name.so" "$gate" --roundtrips > "$proof/unbound-ordinary-name.log" 2>&1
if LD_PRELOAD="$proof/unbound-name.so" "$gate" --names > "$proof/unbound-name.log" 2>&1; then
  printf 'number-sequences: the former interpolated SQL name escaped the gate\n' >&2
  exit 1
fi
rg -q 'zero-length delimited identifier|syntax error|unterminated quoted' "$proof/unbound-name.log"
printf 'number-sequences: 300 cross-process ranges, constant RPCs and serialization/per-value/name controls proved; %s\n' "$proof"
