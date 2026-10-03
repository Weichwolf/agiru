#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
B=${B:-build}
mkdir -p "$B"
proof=$(mktemp -d /tmp/agiru-include-cost.XXXXXX)
CXX=${CXX:-clang++-19}
input_root=$(realpath "${INCLUDE_ROOT:-$PWD}")
rounds=${ROUNDS:-3}
if [[ ! $rounds =~ ^[1-9][0-9]*$ ]]; then
  printf 'include-cost: ROUNDS must be positive\n' >&2
  exit 2
fi
if [ "$#" -eq 0 ]; then set -- RuntimeSurface.h dotnet/Regex.h; fi
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  "-I$input_root/include" "-I$input_root/src/gen")
printf 'source\theader\tround\tfrontend_ms\n' > "$proof/results.tsv"
for header in "$@"; do
  if [[ $header == *'"'* || $header == *$'\n'* ]]; then
    printf 'include-cost: invalid header name\n' >&2
    exit 2
  fi
  label=${header//\//--}
  for ((round = 1; round <= rounds; ++round)); do
    object="$proof/$label-$round.o"
    printf '#include "%s"\n' "$header" | "$CXX" "${flags[@]}" -x c++ - \
      -c -o "$object" -ftime-trace -ftime-trace-granularity=0
    elapsed=$(jq -er '[.traceEvents[] | select(.name == "Total Frontend") | .dur]
      | if length == 1 then .[0] / 1000 else error("missing frontend timing") end' \
      "${object%.o}.json")
    printf '%s\t%s\t%s\t%s\n' "$input_root" "$header" "$round" "$elapsed" \
      >> "$proof/results.tsv"
  done
done
awk -F '\t' 'NR > 1 { sum[$2] += $4; n[$2]++ }
  END { for (header in n) printf "include-cost: %s %.1f ms frontend (%d rounds, no PCH)\n", header, sum[header] / n[header], n[header] }' \
  "$proof/results.tsv"
printf 'include-cost: %s\n' "$proof/results.tsv"
