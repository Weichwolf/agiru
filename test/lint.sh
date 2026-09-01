#!/bin/sh
# `make lint` -- das Format, die statische Analyse, die Tuer.
#
# JEDE BASELINE DARF NUR SCHRUMPFEN. Dieser Baum ist neu, also steht jede heute auf 0 und bleibt
# es: es gibt keine Altlast, fuer die eine Ausnahme zu machen waere. Was hier je ueber null steht,
# ist an dem Tag hineingeschrieben worden.
#
# DIE BASELINE TRAEGT DIE UNIT-ZAHL NEBEN DEM ZAEHLER. Ein Lauf ueber weniger Uebersetzungseinheiten
# findet weniger und wuerde sonst einen falschen Boden festschreiben -- die Falle, die CLAUDE.mds
# Liste als letzte nennt. Faellt die Unit-Zahl, ist das ein ABBRUCH und kein Fortschritt.
set -eu
cd "$(dirname "$0")/.."

TIDY=$(command -v clang-tidy-19 || command -v clang-tidy || true)
FMT=$(command -v clang-format-19 || command -v clang-format || true)
RUNTIDY=$(command -v run-clang-tidy-19 || command -v run-clang-tidy || true)
REPORT=build/lint
BASELINE=test/lint-baseline

[ -n "$FMT" ]  || { echo "lint: clang-format fehlt -- siehe scripts/install.sh" >&2; exit 2; }
[ -n "$TIDY" ] || { echo "lint: clang-tidy fehlt -- siehe scripts/install.sh" >&2; exit 2; }
[ -f compile_commands.json ] || { echo "lint: keine compile_commands.json -- `make db`" >&2; exit 2; }
mkdir -p "$REPORT"

# src/app/ ist Maschinenausgabe. Ein Fund dort hat keine Adresse: der Fix waere im Generator, und
# der Generator wird eine Zeile weiter unten selbst analysiert.
ours=$(find src include test -name '*.cpp' -o -name '*.h' | grep -v '^src/app/' | sort)

printf '== Format ==\n'
if [ -z "$ours" ]; then
  printf 'lint: keine Quelldatei ausserhalb von src/app/ -- nichts zu formatieren\n'
elif "$FMT" --dry-run --Werror $ours 2>"$REPORT/format.log"; then
  printf 'lint: jede Datei ist formatiert\n'
else
  printf 'lint: %s Formatverletzung(en) -- `clang-format -i`, oder %s\n' \
    "$(grep -c ': error:' "$REPORT/format.log" | tr -d ' ')" "$REPORT/format.log" >&2
  exit 1
fi

printf '\n== Analyse ==\n'
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

# EINE ANALYSE, DIE NICHTS FINDET, IST KAPUTT UND NICHT BESTANDEN -- solange es etwas zu finden
# gaebe. In einem leeren Baum gibt es das nicht, also haengt die Wache an der Unit-Zahl und nicht
# am Fund.
read -r allowed allowedUnits <<EOT
$(cat "$BASELINE" 2>/dev/null || echo "0 0")
EOT
allowedUnits=${allowedUnits:-0}
if [ "$units" -lt "$allowedUnits" ]; then
  printf 'lint: die Analyse sah %s Einheiten, zuletzt %s. Ein SCHRUMPFENDER Nenner schreibt einen\n' \
    "$units" "$allowedUnits" >&2
  printf 'lint: falschen Boden fest. Erst den Build reparieren.\n' >&2
  exit 2
fi
sed -n 's/.*\t\[\([a-z0-9-]*\).*/\1/p' "$REPORT/tidy.unique" | sort | uniq -c | sort -rn | head -12
printf '\nlint: %s Fund(e) ueber %s Einheit(en), die Baseline erlaubt %s\n' "$found" "$units" "$allowed"
if [ "$found" -gt "$allowed" ]; then
  printf 'lint: DIE BASELINE IST GEWACHSEN um %s. Ein Commit senkt sie oder laesst sie; nie hebt er sie.\n' \
    "$((found - allowed))" >&2
  printf 'lint: was neu ist, steht in %s\n' "$REPORT/tidy.unique" >&2
  exit 1
fi
if [ "$found" -lt "$allowed" ] || [ "$units" -gt "$allowedUnits" ]; then
  printf '%s %s\n' "$found" "$units" > "$BASELINE"
  printf 'lint: Baseline auf "%s %s" gesetzt -- mit der Reparatur committen.\n' "$found" "$units"
fi

printf '\n== Stille Stellen ==\n'
# EIN NOLINT SCHALTET EINEN FUND AB UND WUERDE SONST NICHTS KOSTEN -- damit waere die Baseline oben
# eine Feige. Jede Stelle, an der dieser Baum eine Meldung unterdrueckt oder einen Fehler schluckt,
# traegt hier eine Zahl, und die darf nur fallen.
silent=$(grep -rn 'NOLINT\|TODO\|FIXME\|catch (\.\.\.)' src include test --include='*.cpp' \
  --include='*.h' 2>/dev/null | grep -v '^src/app/' | wc -l | tr -d ' ')
allowedSilent=$(cat test/todo-baseline 2>/dev/null || echo 0)
printf 'lint: %s stille Stelle(n), die Baseline erlaubt %s\n' "$silent" "$allowedSilent"
if [ "$silent" -gt "$allowedSilent" ]; then
  printf 'lint: EINE STILLE STELLE IST DAZUGEKOMMEN. Sie traegt ihren Grund in der Zeile darueber,\n' >&2
  printf 'lint: oder sie geht wieder weg. Die Baseline hebt sich nicht von selbst.\n' >&2
  grep -rn 'NOLINT\|TODO\|FIXME\|catch (\.\.\.)' src include test --include='*.cpp' --include='*.h' \
    2>/dev/null | grep -v '^src/app/' >&2
  exit 1
fi
if [ "$silent" -lt "$allowedSilent" ]; then
  printf '%s\n' "$silent" > test/todo-baseline
  printf 'lint: Baseline auf %s gesenkt -- mit der Reparatur committen.\n' "$silent"
fi
