#!/bin/sh
# `make lint` -- the format, the static analysis, the door.
#
# EVERY BASELINE MAY ONLY SHRINK. This tree is new, so each stands at 0 today and stays there:
# there is no legacy to make an exception for. Anything above zero here was written in on the day.
#
# THE BASELINE CARRIES THE UNIT COUNT BESIDE THE COUNTER. A run over fewer translation units finds
# fewer and would otherwise write a false floor -- the last trap on CLAUDE.md's list. A shrinking
# unit count is an ABORT, not progress.
set -eu
cd "$(dirname "$0")/.."

TIDY=$(command -v clang-tidy-19 || command -v clang-tidy || true)
FMT=$(command -v clang-format-19 || command -v clang-format || true)
RUNTIDY=$(command -v run-clang-tidy-19 || command -v run-clang-tidy || true)
REPORT=build/lint
BASELINE=test/lint-baseline

[ -n "$FMT" ]  || { echo "lint: clang-format is missing -- see scripts/install.sh" >&2; exit 2; }
[ -n "$TIDY" ] || { echo "lint: clang-tidy is missing -- see scripts/install.sh" >&2; exit 2; }
[ -f compile_commands.json ] || { echo "lint: no compile_commands.json -- run \`make db\`" >&2; exit 2; }
mkdir -p "$REPORT"

# src/app/ is machine output. A finding there has no address: the fix would be in the generator,
# and the generator is analysed a line further down.
ours=$(find src include test -name '*.cpp' -o -name '*.h' | grep -v '^src/app/' | sort)

printf '== format ==\n'
if [ -z "$ours" ]; then
  printf 'lint: no source outside src/app/ -- nothing to format\n'
elif "$FMT" --dry-run --Werror $ours 2>"$REPORT/format.log"; then
  printf 'lint: every file is formatted\n'
else
  printf 'lint: %s formatting violation(s) -- `clang-format -i`, or %s\n' \
    "$(grep -c ': error:' "$REPORT/format.log" | tr -d ' ')" "$REPORT/format.log" >&2
  exit 1
fi

printf '\n== analysis ==\n'
units=$(grep -c '"file"' compile_commands.json || echo 0)
if [ -n "$RUNTIDY" ]; then
  "$RUNTIDY" -p . -quiet -j "$(nproc)" '^(?!.*/src/app/).*\.cpp$' > "$REPORT/tidy.log" 2>&1 || true
else
  : > "$REPORT/tidy.log"
  for f in $(echo "$ours" | grep '\.cpp$'); do
    "$TIDY" -p . --quiet "$f" >> "$REPORT/tidy.log" 2>&1 || true
  done
fi
grep 'warning:\|error:' "$REPORT/tidy.log" | sed 's/ \[/\t[/' | sort -u > "$REPORT/tidy.unique"
found=$(wc -l < "$REPORT/tidy.unique" | tr -d ' ')

# AN ANALYSIS THAT FINDS NOTHING IS BROKEN, NOT PASSED -- as long as there is something to find.
# In an empty tree there is not, so the guard hangs off the unit count rather than the findings.
read -r allowed allowedUnits <<EOT
$(cat "$BASELINE" 2>/dev/null || echo "0 0")
EOT
allowedUnits=${allowedUnits:-0}
if [ "$units" -lt "$allowedUnits" ]; then
  printf 'lint: the analysis saw %s unit(s), last time %s. A SHRINKING denominator writes a\n' \
    "$units" "$allowedUnits" >&2
  printf 'lint: false floor. Repair the build first.\n' >&2
  exit 2
fi
sed -n 's/.*\t\[\([a-z0-9-]*\).*/\1/p' "$REPORT/tidy.unique" | sort | uniq -c | sort -rn | head -12
printf '\nlint: %s finding(s) over %s unit(s), the baseline allows %s\n' "$found" "$units" "$allowed"
if [ "$found" -gt "$allowed" ]; then
  printf 'lint: THE BASELINE GREW by %s. A commit lowers it or leaves it; it never raises it.\n' \
    "$((found - allowed))" >&2
  printf 'lint: what is new is in %s\n' "$REPORT/tidy.unique" >&2
  exit 1
fi
if [ "$found" -lt "$allowed" ] || [ "$units" -gt "$allowedUnits" ]; then
  printf '%s %s\n' "$found" "$units" > "$BASELINE"
  printf 'lint: baseline set to "%s %s" -- commit it with the repair.\n' "$found" "$units"
fi

printf '\n== silent places ==\n'
# A NOLINT SWITCHES A FINDING OFF AND WOULD OTHERWISE COST NOTHING -- which would make the baseline
# above a fig leaf. Every place where this tree suppresses a diagnostic or swallows an error carries
# a number here, and that number may only fall.
silent=$(grep -rn 'NOLINT\|TODO\|FIXME\|catch (\.\.\.)' src include test --include='*.cpp' \
  --include='*.h' 2>/dev/null | grep -v '^src/app/' | wc -l | tr -d ' ')
allowedSilent=$(cat test/todo-baseline 2>/dev/null || echo 0)
printf 'lint: %s silent place(s), the baseline allows %s\n' "$silent" "$allowedSilent"
if [ "$silent" -gt "$allowedSilent" ]; then
  printf 'lint: A SILENT PLACE WAS ADDED. It carries its reason in the line above it,\n' >&2
  printf 'lint: or it goes away again. The baseline does not raise itself.\n' >&2
  grep -rn 'NOLINT\|TODO\|FIXME\|catch (\.\.\.)' src include test --include='*.cpp' --include='*.h' \
    2>/dev/null | grep -v '^src/app/' >&2
  exit 1
fi
if [ "$silent" -lt "$allowedSilent" ]; then
  printf '%s\n' "$silent" > test/todo-baseline
  printf 'lint: baseline lowered to %s -- commit it with the repair.\n' "$silent"
fi
