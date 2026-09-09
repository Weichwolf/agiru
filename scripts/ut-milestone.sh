#!/bin/bash
# THE UT MILESTONE, MEASURED IN PARALLEL. One process per codeunit against its own clone of the
# seeded template (`--fresh --scratch`), N at a time; the clone is dropped afterwards. Four workers
# measured the whole suite in 691 s where one took some 45 min, with the SAME per-codeunit
# answers (2026-09-09) -- which is the multi-user claim of this runtime, tested every round.
#
# THE DENOMINATOR IS COUNTED FROM THE TEXT AND NEVER FROM THE PARSER: every `.Codeunit.al` under
# the W1 tests with `Subtype = Test` whose name ends in ` UT`, `-UT` or `.UT` (80 on 2026-09-09).
# A codeunit that prints no total is LOST and stays in the denominator; a run that finds no
# codeunit at all is an ABORT and not a pass.
#
#   scripts/ut-milestone.sh <out.log> [workers=4] [dsn=postgresql://agiru:agiru@localhost:5433/agiru_seeded]
set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT" || exit 2
out="$1"; workers="${2:-4}"; dsn="${3:-postgresql://agiru:agiru@localhost:5433/agiru_seeded}"
TESTS="${AGIRU_BC_SOURCE:-$HOME/Git/BCApps/src}/Layers/W1/Tests"
parts=$(mktemp -d)
list="$parts/codeunits.txt"
grep -rlZ "Subtype = Test" "$TESTS" --include='*.Codeunit.al' \
  | xargs -0 grep -h -m1 -oE '^codeunit [0-9]+ "[^"]*( UT|-UT|\.UT)"' \
  | sed -E 's/^codeunit [0-9]+ "//; s/"$//' | sort > "$list"
if [ ! -s "$list" ]; then
  printf 'ut-milestone: no UT codeunit found under %s -- ABORT, not a pass\n' "$TESTS" >&2
  exit 1
fi
one() {
  name="$1"; scratch="agiru_ut_$$"
  log=$(timeout 900 ./build/agiru run-tests --database "$DSN" --fresh --scratch "$scratch" \
        --codeunit "$name" 2>&1)
  status=$?
  podman exec agiru-pg psql -U agiru -tAc "DROP DATABASE IF EXISTS \"$scratch\"" >/dev/null 2>&1
  key=$(printf '%s' "$name" | tr -c 'A-Za-z0-9' '_')
  if printf '%s\n' "$log" | grep -qE "^[0-9]+ of [0-9]+ passed"; then
    printf '%s\n' "$log" > "$PARTS/$key.log"
  else
    printf 'LOST %s (exit %d)\n' "$name" "$status" > "$PARTS/$key.log"
  fi
}
export -f one; export PARTS="$parts" DSN="$dsn"
start=$(date +%s)
xargs -P "$workers" -I{} bash -c 'one "$@"' _ {} < "$list"
: > "$out"
pass=0; total=0; ran=0; lost=0
while IFS= read -r name; do
  key=$(printf '%s' "$name" | tr -c 'A-Za-z0-9' '_')
  f="$parts/$key.log"
  if [ ! -f "$f" ] || grep -q '^LOST ' "$f"; then
    { [ -f "$f" ] && cat "$f" || printf 'LOST %s (exit ?)\n' "$name"; } >> "$out"
    lost=$((lost + 1)); continue
  fi
  line=$(grep -E "^[0-9]+ of [0-9]+ passed" "$f" | tail -1)
  p=${line%% of *}; rest=${line#* of }; t=${rest%% passed*}
  pass=$((pass + p)); total=$((total + t)); ran=$((ran + 1))
  cat "$f" >> "$out"
done < "$list"
rm -rf "$parts"
printf 'UT MILESTONE: %d of %d over %d codeunits, %d that printed no total (%d workers, %d s)\n' \
  "$pass" "$total" "$ran" "$lost" "$workers" "$(( $(date +%s) - start ))" | tee -a "$out"
