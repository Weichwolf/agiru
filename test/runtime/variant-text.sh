#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-variant-text.XXXXXX)
trap 'find "$proof" -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.so" \) -delete' EXIT
printf 'variant-text: receipts %s\n' "$proof"
sha256sum test/runtime/variant-text.sh include/type/Variant.h src/net/Variant.cpp \
  src/rt/written/BuiltinsWritten.cpp test/gate/{Variant,Format,RecordRef}Gate.cpp \
  "$B/libagiru_net.so" "$B/libagiru_rt.so" > "$proof/inputs.sha256"
for name in Variant Format RecordRef; do
  "$B/gate_${name}Gate" | tee "$proof/$name.log"
done
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude -Isrc/net --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
for control in caption blank variant; do
  awk -v control="$control" '
    control == "caption" && /return std::string\(member->caption.empty\(\)/ {
      $0="  return std::string(member->name);"; changed++
    }
    control == "blank" && /return std::string\(member->caption.empty\(\)/ {
      print "  if (member->name.empty() && member->caption.empty()) { return std::to_string(ordinal); }";
      changed++
    }
    control == "variant" && /std::string_view Variant::Rendered\(\) const \{/ {
      print; print "  if (IsOption()) { Refuse(\"Text\"); }"; changed++; next
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/net/Variant.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_net -o "$proof/$control.so"
  consumers=(Variant Format)
  if [ "$control" = variant ]; then consumers=(Variant RecordRef); fi
  for name in "${consumers[@]}"; do
    status=0
    LD_PRELOAD="$proof/$control.so" "$B/gate_${name}Gate" \
      > "$proof/$control-$name.log" 2>&1 || status=$?
    if [ "$status" -ne 1 ]; then
      printf 'variant-text: %s control escaped %s or failed unexpectedly (exit %s)\n' \
        "$control" "$name" "$status" >&2
      exit 1
    fi
    rg -q 'FAIL ' "$proof/$control-$name.log"
  done
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
printf 'variant-text: shared display text and typed identity pass; three compiled controls reject in both consumers; %s\n' "$proof"
