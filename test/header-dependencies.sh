#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
B=${B:-build}
mkdir -p "$B"
proof=$(mktemp -d "$B/header-dependencies.XXXXXX")
CXX=${CXX:-clang++-19}
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/gen)

compile_header() {
  local header=$1
  local dependencies=$2
  shift 2
  printf '#include "%s"\n' "$header" | "$CXX" "${flags[@]}" "$@" \
    -x c++ - -fsyntax-only -MD -MF "$dependencies" -MT header-probe
}

reject_dependency() {
  local dependencies=$1
  local forbidden=$2
  if rg -q "/$forbidden([[:space:]\\\\]|$)" "$dependencies"; then
    printf 'header-dependencies: %s contains <%s>\n' "$dependencies" "$forbidden" >&2
    return 1
  fi
}

for header in ObjectKind.h RuntimeSurface.h Names.h; do
  compile_header "$header" "$proof/$header.d"
  reject_dependency "$proof/$header.d" filesystem
done
compile_header dotnet/Regex.h "$proof/Regex.h.d"
reject_dependency "$proof/Regex.h.d" regex

for forbidden in filesystem regex; do
  if [ "$forbidden" = filesystem ]; then header=RuntimeSurface.h; else header=dotnet/Regex.h; fi
  compile_header "$header" "$proof/forced-$forbidden.d" -include "$forbidden"
  if reject_dependency "$proof/forced-$forbidden.d" "$forbidden" \
    > "$proof/forced-$forbidden.log" 2>&1; then
    printf 'header-dependencies: forced <%s> escaped the control\n' "$forbidden" >&2
    exit 1
  fi
done
printf 'header-dependencies: four standalone headers; filesystem/regex controls refused\n'
