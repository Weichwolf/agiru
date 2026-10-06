#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-variant-text.XXXXXX)
trap 'find "$proof" -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.so" \) -delete' EXIT
printf 'variant-text: receipts %s\n' "$proof"
sha256sum test/runtime/variant-text.sh include/type/Variant.h src/net/Variant.cpp \
  include/runtime/Report.h src/rt/Report.cpp src/rt/written/BuiltinsWritten.cpp \
  test/gate/{Variant,Format,RecordRef,Report}Gate.cpp \
  "$B/libagiru_net.so" "$B/libagiru_rt.so" > "$proof/inputs.sha256"
for name in Variant Format RecordRef Report; do
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
  consumers=(Variant Format Report)
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
for control in report-numeric report-display-scalars; do
  awk -v control="$control" '
    /const auto format = value.IsOption\(\)/ {
      if (control == "report-numeric") print "  const auto format = false ? kDisplayFormat : kXmlFormat;";
      else print "  const auto format = true ? kDisplayFormat : kXmlFormat;";
      changed++; next
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/Report.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" -Isrc/rt "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_rt -lagiru_net -o "$proof/$control.so"
  status=0
  LD_PRELOAD="$proof/$control.so" "$B/gate_ReportGate" \
    > "$proof/$control-Report.log" 2>&1 || status=$?
  if [ "$status" -ne 1 ]; then
    printf 'variant-text: %s control escaped Report or failed unexpectedly (exit %s)\n' \
      "$control" "$status" >&2
    exit 1
  fi
  if [ "$control" = report-numeric ]; then
    rg -q 'FAIL .* an ordinal report column carries escaped declared display text' \
      "$proof/$control-Report.log"
  else
    rg -q 'FAIL .* Boolean report columns retain XML rather than display formatting' \
      "$proof/$control-Report.log"
  fi
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
sha256sum --check "$proof/inputs.sha256" > "$proof/input-integrity.log"
printf 'variant-text: shared display text, report XML scalars and typed identity pass; five compiled controls reject across their consumers; %s\n' "$proof"
