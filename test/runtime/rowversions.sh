#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-rowversions.XXXXXX)
gate="$B/gate_RowVersionGate"
record_gate="$B/gate_SqlRowVersionGate"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -fPIC -shared -Iinclude
  --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_db)
trap 'find "$proof" -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.so" \) -delete' EXIT
sha256sum src/rt/RowVersionStorage.cpp src/rt/{Storage,SqlColumn,Table,Query,Where,Selection,Navigate}.cpp \
  src/rt/SqlColumn.h src/rt/Selection.h src/rt/Rows.h include/runtime/{Storage,Table,RecordRef}.h test/gate/{RowVersionGate,SqlRowVersionGate}.cpp \
  test/gate/OwnedDatabase.h "$gate" "$record_gate" "$B/libagiru_rt.so" > "$proof/inputs.sha256"
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

"$record_gate" > "$proof/sql-record-baseline.log" 2>&1
gate="$record_gate"
flags+=(-Isrc/rt)

awk '
  / = agiru_platform.next_rowversion_v1\(\)";/ { sub(/next_rowversion_v1\(\)/, "last_rowversion_v1()"); matches++ }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/Storage.cpp > "$proof/reused-update-version.cpp"
build_overlay reused-update-version
expect_red reused-update-version "SystemRowVersion is the stored platform version"

awk '
  /SetFieldText\(record, def, Required\(\(\*owned\)/ {
    print "    if (def.sqlTimestamp) { ++column; continue; }"; matches++
  }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/Table.cpp > "$proof/stale-record-buffer.cpp"
build_overlay stale-record-buffer
expect_red stale-record-buffer "SystemRowVersion is the stored platform version"

awk '
  /return Quoted\(ColumnName\(field\)\)/ { sub(/ColumnName\(field\)/, "field.name"); matches++ }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/SqlColumn.cpp > "$proof/source-name-column.cpp"
build_overlay source-name-column
expect_red source-name-column 'column "Version" does not exist'

awk '
  /return Where\(def, expr, first, SqlColumn\(def\)\)/ {
    sub(/SqlColumn\(def\)/, "SqlColumn(FieldDef{.name = def.name})"); matches++
  }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/Where.cpp > "$proof/source-name-filter.cpp"
build_overlay source-name-filter
expect_red source-name-filter 'column "Version" does not exist'

awk '
  /return Alias\(column.dataItem\) .*SqlColumn\(FieldIn/ {
    sub(/SqlColumn\(FieldIn\(def, item, column.field\)\)/, "Quoted(FieldIn(def, item, column.field).name)"); matches++
  }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/Query.cpp > "$proof/source-name-query.cpp"
build_overlay source-name-query
expect_red source-name-query 'column d0.Version does not exist'

awk '
  /SqlColumn\(\*column.field\)/ {
    sub(/SqlColumn\(\*column.field\)/, "Quoted(column.field->name)"); matches++
  }
  /const std::string name = SqlColumn\(def\)/ {
    sub(/SqlColumn\(def\)/, "Quoted(def.name)"); matches++
  }
  { print }
  END { if (matches != 3) exit 2 }
' src/rt/Navigate.cpp > "$proof/source-name-navigation.cpp"
build_overlay source-name-navigation
expect_red source-name-navigation 'column "Version" does not exist'

awk '
  /field.sqlTimestamp \? std::string\("agiru_platform.next_rowversion_v1\(\)"\)/ {
    sub(/agiru_platform.next_rowversion_v1\(\)/, "0"); matches++
  }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/Storage.cpp > "$proof/zero-backfill.cpp"
build_overlay zero-backfill
expect_red zero-backfill "migration allocates one nonzero version per existing row"

"$record_gate" > "$proof/sql-record-restored.log" 2>&1

awk '
  /^bool RuntimeGetBySystemId\(/ { lookup=1 }
  /^void RuntimeSetRecFilter\(/ { lookup=0 }
  lookup && /state.positioned = true;/ { matches++; next }
  { print }
  END { if (matches != 1) exit 2 }
' src/rt/Table.cpp > "$proof/unpositioned-system-id.cpp"
build_overlay unpositioned-system-id
expect_red unpositioned-system-id "SystemId lookup records a current position"

mkdir -p "$proof/unchecked-system-id/runtime"
awk '
  /return \{found, table, std::move\(key\)\};/ {
    sub(/\{found,/, "{true,"); matches++
  }
  { print }
  END { if (matches != 1) exit 2 }
' include/runtime/Table.h > "$proof/unchecked-system-id/runtime/Table.h"
"$CXX" -O2 -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
  --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
  "-DAGIRU_TEST_DSN=\"${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}\"" \
  "-I$proof/unchecked-system-id" -Iinclude -Itest/gate \
  test/gate/SqlRowVersionGate.cpp \
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db \
  -o "$proof/unchecked-system-id/gate" > "$proof/unchecked-system-id.compile.log" 2>&1
status=0
"$proof/unchecked-system-id/gate" > "$proof/unchecked-system-id.log" 2>&1 || status=$?
[ "$status" -eq 1 ]
rg -q "FAIL .*discarding a typed missing SystemId result" "$proof/unchecked-system-id.log"
rg -q "FAIL .*discarding a reflected missing SystemId result" "$proof/unchecked-system-id.log"
find "$proof/unchecked-system-id" -depth -delete

sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
printf 'rowversions: allocator fences, SQL record/alias/SystemId paths and seventeen compiled negative controls proved; %s\n' "$proof"
