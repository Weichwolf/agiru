#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-reflection-metadata.XXXXXX)
gate="$B/gate_ReflectionMetadataGate"
"$gate" > "$proof/current.log" 2>&1
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude -Isrc/rt --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)

for control in ordinal-cast cds-query default-fallback; do
  awk -v control="$control" '
    control == "ordinal-cast" && /case PageType::HeadlinePart: return Native::HeadlinePart;/ {
      sub(/return Native::HeadlinePart;/, "return static_cast<Native>(type);"); changed++
    }
    control == "cds-query" && /case TableType::CDS:/ {
      print "    case TableType::CDS: return Native::Query;"; changed++; skipping=1; next
    }
    skipping { if (/;$/) skipping=0; next }
    control == "default-fallback" && /return std::unexpected\("unknown TableType/ {
      $0 = "  return Native::Normal;"; changed++
    }
    { print }
    END { if (changed != 1 || skipping) exit 2 }
  ' src/rt/ReflectionMetadata.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1; then
    printf 'reflection-metadata: %s escaped the identity/refusal gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
done

awk '
  /^void RequireTableProvider\(const TableDef &table\) \{$/ {
    print "void RequireTableProvider(const TableDef &) {}"; skipping=1; changed++; next
  }
  skipping { if (/^}$/) skipping=0; next }
  { print }
  END { if (changed != 1 || skipping) exit 2 }
' src/rt/Storage.cpp > "$proof/UncheckedStorage.cpp"
"$CXX" "${flags[@]}" "$proof/UncheckedStorage.cpp" -L"$B" -Wl,-rpath,"$B" \
  -lagiru_rt -lagiru_net -lagiru_db -o "$proof/unchecked-storage.so"
if LD_PRELOAD="$proof/unchecked-storage.so" "$gate" > "$proof/unchecked-storage.log" 2>&1; then
  printf 'reflection-metadata: unqualified physical metadata access escaped the guard\n' >&2
  exit 1
fi
rg -q 'unqualified live metadata refuses' "$proof/unchecked-storage.log"
printf 'reflection-metadata: typed projection and temporary rows pass; four controls refuse; %s\n' "$proof"
