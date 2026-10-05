#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-rowversions.XXXXXX)
gate="$B/gate_RowVersionGate"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -fPIC -shared -Iinclude
  --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_db)
trap 'find "$proof" -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.so" \) -delete' EXIT
sha256sum src/rt/RowVersionStorage.cpp test/gate/RowVersionGate.cpp "$gate" > "$proof/inputs.sha256"
"$CXX" --version > "$proof/compiler.txt"

build_overlay() {
  "$CXX" "$proof/$1.cpp" "${flags[@]}" -o "$proof/$1.so"
}

expect_red() {
  local overlay=$1 claim=$2
  shift 2
  local status=0
  LD_PRELOAD="$proof/$overlay.so" "$gate" "$@" > "$proof/$overlay.log" 2>&1 || status=$?
  if [ "$status" -ne 1 ]; then
    printf 'rowversions: %s expected gate failure 1, received %s\n' "$overlay" "$status" >&2
    exit 1
  fi
  rg -q "$claim" "$proof/$overlay.log"
}

"$gate" > "$proof/baseline.log" 2>&1

awk '
  /PERFORM pg_catalog.pg_advisory_xact_lock\(-value\);/ { matches++; next }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/RowVersionStorage.cpp > "$proof/missing-fence.cpp"
build_overlay missing-fence
expect_red missing-fence "active minimum retains the older writer"

awk '
  /SELECT pg_catalog.min\(-\(\(classid/ { print "    value := NULL;"; skip=1; matches++; next }
  skip { if (/WHERE datname = pg_catalog.current_database\(\)\);/) skip=0; next }
  { print }
  END { if (matches != 1 || skip) exit 2 }
' src/rt/RowVersionStorage.cpp > "$proof/last-plus-one.cpp"
build_overlay last-plus-one
expect_red last-plus-one "newer commit cannot hide the older uncommitted writer"

awk '
  /IF pg_catalog.starts_with\(fence, transaction_tag\) THEN/ { skip=3; matches++ }
  skip { skip--; next }
  { print }
  END { if (matches != 1 || skip) exit 2 }
' src/rt/RowVersionStorage.cpp > "$proof/per-row-fence.cpp"
build_overlay per-row-fence
expect_red per-row-fence "a thousand row writes retain only one transaction fence"

awk '
  /AND database = \(SELECT oid FROM pg_catalog.pg_database/ { skip=1; matches++; next }
  skip { if (!/WHERE datname = pg_catalog.current_database\(\)\);/) exit 2; print "        ;"; skip=0; next }
  { print }
  END { if (matches != 1 || skip) exit 2 }
' src/rt/RowVersionStorage.cpp > "$proof/cross-database.cpp"
build_overlay cross-database
expect_red cross-database "cluster-wide locks do not lower another database"

awk '
  /CACHE 1 NO CYCLE/ { sub(/CACHE 1/, "CACHE 32"); creation++ }
  /properties.seqcache = 1/ { sub(/seqcache = 1/, "seqcache = 32"); validation++ }
  { print }
  END { if (creation != 1 || validation != 1) exit 2 }
' src/rt/RowVersionStorage.cpp > "$proof/cached-sequence.cpp"
build_overlay cached-sequence
expect_red cached-sequence "last used includes uncommitted allocations"

awk '
  { print }
  /value := pg_catalog.nextval/ { print "    PERFORM pg_catalog.pg_sleep(0.5);"; matches++ }
  END { if (matches != 1) exit 2 }
' src/rt/RowVersionStorage.cpp > "$proof/paused-publisher.cpp"
build_overlay paused-publisher
LD_PRELOAD="$proof/paused-publisher.so" "$gate" --publication > "$proof/publication.log" 2>&1
LD_PRELOAD="$proof/paused-publisher.so" "$gate" --cancellation > "$proof/cancellation.log" 2>&1

awk '
  /constexpr std::string_view kNext/ { allocator=1 }
  /constexpr std::string_view kMinimum/ { allocator=0 }
  allocator && /pg_advisory_lock\(x/ { locks++; next }
  allocator && /pg_advisory_unlock\(x/ { unlocks++; next }
  { print }
  END { if (locks != 1 || unlocks != 3) exit 2 }
' "$proof/paused-publisher.cpp" > "$proof/unpublished.cpp"
build_overlay unpublished
expect_red unpublished "minimum cannot pass an unpublished allocation" --publication

awk '
  /constexpr std::string_view kNext/ { allocator=1 }
  /constexpr std::string_view kMinimum/ { allocator=0 }
  allocator && /WHEN query_canceled OR assert_failure THEN/ { print; skip=1; matches++; next }
  skip { if (!/pg_advisory_unlock/) exit 2; skip=0; next }
  { print }
  END { if (matches != 1 || skip) exit 2 }
' "$proof/paused-publisher.cpp" > "$proof/cancellation-leak.cpp"
build_overlay cancellation-leak
expect_red cancellation-leak "lock timeout" --cancellation

awk '
  /IF value IS NULL THEN/ { print "    PERFORM pg_catalog.pg_sleep(0.5);"; matches++ }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/RowVersionStorage.cpp > "$proof/paused-minimum.cpp"
build_overlay paused-minimum
LD_PRELOAD="$proof/paused-minimum.so" "$gate" --minimum-cancellation > "$proof/minimum-cancellation.log" 2>&1

awk '
  /constexpr std::string_view kMinimum/ { minimum=1 }
  minimum && /WHEN query_canceled OR assert_failure THEN/ { print; skip=1; matches++; next }
  skip { if (!/pg_advisory_unlock/) exit 2; skip=0; next }
  { print }
  END { if (matches != 1 || skip) exit 2 }
' "$proof/paused-minimum.cpp" > "$proof/minimum-cancellation-leak.cpp"
build_overlay minimum-cancellation-leak
expect_red minimum-cancellation-leak "lock timeout" --minimum-cancellation

"$gate" > "$proof/restored.log" 2>&1
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
printf 'rowversions: fences, bounds, cross-database isolation, publication and cancellation controls proved; %s\n' "$proof"
