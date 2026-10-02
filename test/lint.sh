#!/bin/sh
# `make lint` -- the format, the static analysis, the door.
#
# EVERY BASELINE MAY ONLY SHRINK. This tree is new, so each stands at 0 today and stays there:
# there is no legacy to make an exception for. Anything above zero here was written in on the day.
#
# THE BASELINE CARRIES THE UNIT COUNT BESIDE THE COUNTER. A run over fewer translation units finds
# fewer and would otherwise write a false floor -- the last trap on AGENTS.md's list. A shrinking
# unit count is an ABORT, not progress.
set -eu
FULL=${FULL:-}
[ "$FULL" != 0 ] || FULL=
cd "$(dirname "$0")/.."

TIDY=$(command -v clang-tidy-19 || command -v clang-tidy || true)
FMT=$(command -v clang-format-19 || command -v clang-format || true)
REPORT=build/lint
BASELINE=test/lint-baseline

[ -n "$FMT" ]  || { echo "lint: clang-format is missing -- see scripts/install.sh" >&2; exit 2; }
[ -n "$TIDY" ] || { echo "lint: clang-tidy is missing -- see scripts/install.sh" >&2; exit 2; }
[ -f compile_commands.json ] || { echo "lint: no compile_commands.json -- run \`make db\`" >&2; exit 2; }
mkdir -p "$REPORT"

# apps/ IS MACHINE OUTPUT AND NOTHING HERE LOOKS AT IT -- not the formatter, not clang-tidy. A
# finding there has no address: nobody edits the file, so the repair would be in the generator, and
# the generator is analysed a line further down. The formatter never reaches it because the roots
# below do not include it; clang-tidy never reaches it because of the pattern below; and the
# generated tree carries its own formatting already, since the emitter pipes every file through
# clang-format on the way out.
ours=$(find src include test -name '*.cpp' -o -name '*.h' | sort)

printf '== format ==\n'
if [ -z "$ours" ]; then
  printf 'lint: no hand-written source -- nothing to format\n'
elif "$FMT" --dry-run --Werror $ours 2>"$REPORT/format.log"; then
  printf 'lint: every file is formatted\n'
else
  printf 'lint: %s formatting violation(s) -- `clang-format -i`, or %s\n' \
    "$(grep -c ': error:' "$REPORT/format.log" | tr -d ' ')" "$REPORT/format.log" >&2
  exit 1
fi

printf '\n== analysis ==\n'
NODES=50000
[ "${DEEP:-}" != 1 ] || NODES=225000
analysis_status=0
python3 test/lint-analysis.py --tidy "$TIDY" ${FULL:+--full} \
  --jobs "${JOBS:-$(nproc)}" --nodes "$NODES" --padding "${PAD:-64}" || analysis_status=$?
[ "$analysis_status" -ne 2 ] || exit 2
grep 'warning:\|error:' "$REPORT/tidy.log" | sed 's/ \[/\t[/' | sort -u > "$REPORT/tidy.unique"
found=$(wc -l < "$REPORT/tidy.unique" | tr -d ' ')
units=$(python3 -c 'import json; print(json.load(open("build/lint/units.json"))["checked"])')
if [ "$analysis_status" -ne 0 ]; then
  cat "$REPORT/tidy.unique" >&2
  printf 'lint: clang-tidy failed; full output is in %s/tidy.log\n' "$REPORT" >&2
  exit "$analysis_status"
fi
if [ -z "${FULL:-}" ] && [ "$found" -ne 0 ]; then
  cat "$REPORT/tidy.unique" >&2
  exit 1
fi
# A SUPPRESSION IS A DIRECTIVE, NOT A WORD IN A COMMENT, and the difference is what this counter
# got wrong. It dropped every line whose content began with `/` or `*` -- to skip the prose that
# EXPLAINS the rule -- and `NOLINTNEXTLINE` is written on a comment line of its own, which is how
# clang-tidy wants it and how all seven suppressions in this tree are written. So the counter saw
# one of seven and reported zero for months of them. Matching the DIRECTIVE form instead
# (`NOLINT`, `NOLINTNEXTLINE`, `NOLINTBEGIN`, `NOLINTEND`, each followed by `(`, a space or the end
# of the line) counts a suppression wherever it stands, and a backticked mention in prose is
# excluded by what it is rather than by where it sits.
grep_silent() {
  grep -rnE 'NOLINT(NEXTLINE|BEGIN|END)?(\(|$| )|TODO|FIXME|catch \(\.\.\.\) *\{ *\}' \
    src include test --include='*.cpp' --include='*.h' 2>/dev/null |
    grep -v '`NOLINT' | grep -v '`TODO' | grep -v '`FIXME' | grep -v '^apps/'
}

printf '\n== silent places ==\n'
silent=$(grep_silent | wc -l | tr -d ' ')
allowedSilent=$(cat test/todo-baseline)
printf 'lint: %s silent place(s), baseline %s\n' "$silent" "$allowedSilent"
if [ "$silent" -gt "$allowedSilent" ]; then
  grep_silent >&2
  exit 1
fi
if [ -z "$FULL" ]; then
  printf '\nlint: changed-code checks passed; full surface checks require FULL=1.\n'
  exit 0
fi

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
# above a fig leaf. Every place where this tree suppresses a diagnostic or SWALLOWS an error carries
# a number here, and that number may only fall.
#
# WHAT COUNTS AND WHAT DOES NOT, because the first version of this counter measured the wrong thing.
# It counted every `catch (...)`, which made a handler that REPORTS and returns non-zero cost the
# same as one that eats the error -- and it counted the word inside a comment about the word. What
# is silent is an EMPTY handler. A handler that says what went wrong is the opposite of silent, and
# charging for it pushed the tree toward having none.
silent=$(grep_silent | wc -l | tr -d ' ')
allowedSilent=$(cat test/todo-baseline 2>/dev/null || echo 0)
printf 'lint: %s silent place(s), the baseline allows %s\n' "$silent" "$allowedSilent"
if [ "$silent" -gt "$allowedSilent" ]; then
  printf 'lint: A SILENT PLACE WAS ADDED. It carries its reason in the line above it,\n' >&2
  printf 'lint: or it goes away again. The baseline does not raise itself.\n' >&2
  grep_silent >&2
  exit 1
fi
if [ "$silent" -lt "$allowedSilent" ]; then
  printf '%s\n' "$silent" > test/todo-baseline
  printf 'lint: baseline lowered to %s -- commit it with the repair.\n' "$silent"
fi

printf '\n== the door ==\n'
# EVERY PUBLIC NAME IS DOCUMENTED OR IT IS A WARNING. `include/` is the public interface, and
# a public name without a sign on it is the one thing a reader cannot recover from the code.
if command -v doxygen >/dev/null 2>&1; then
  mkdir -p build/doc
  doxygen doc/Doxyfile >"$REPORT/doxygen.log" 2>&1 || { cat "$REPORT/doxygen.log" >&2; exit 2; }
  # ONE WARNING CLASS IS DOXYGEN'"'"'S OWN LIMIT AND NOT A MISSING SIGN. `Text<N> : Text<0>` and
  # `Code<N> : Code<0>` are legal C++ -- a primary template deriving from its own specialisation --
  # and doxygen reports each of them as a "potential recursive class relation". The construct is
  # what lets AL hand a `Text[30]` to a `var Text` parameter, so the finding argues with AL rather
  # than with us; every other warning still counts, including a name in the same file.
  test -f build/doc/warnings.txt || { echo "lint: missing doxygen warnings file" >&2; exit 2; }
  grep -v "recursive class relation" build/doc/warnings.txt > build/doc/undocumented.txt || test "$?" -eq 1
  undocumented=$(wc -l < build/doc/undocumented.txt 2>/dev/null | tr -d ' ')
  allowedDoc=$(cat test/doc-baseline 2>/dev/null || echo 0)
  printf 'lint: %s undocumented public entit(ies), the baseline allows %s\n' \
    "$undocumented" "$allowedDoc"
  if [ "$undocumented" -gt "$allowedDoc" ]; then
    printf 'lint: THE DOOR BASELINE GREW by %s -- a public name arrived without a sign.\n' \
      "$((undocumented - allowedDoc))" >&2
    printf 'lint: they are named in build/doc/undocumented.txt\n' >&2
    exit 1
  fi
  if [ "$undocumented" -lt "$allowedDoc" ]; then
    printf '%s\n' "$undocumented" > test/doc-baseline
    printf 'lint: door baseline lowered to %s -- commit it with the repair.\n' "$undocumented"
  fi
else
  printf 'lint: doxygen is required for FULL=1.\n' >&2
  exit 2
fi

printf '\n== the documented triggers ==\n'
# THE COUNT OF NAMED TRIGGERS IS A BASELINE THAT MAY ONLY RISE (board:0588): a documented trigger
# the tree never names is fired by nothing, declared by nothing and refused by nothing.
python3 test/triggers.py --record || exit 1

printf '\n== the AL population ==\n'
# THE COUNT OF TRANSLATED OBJECTS IS A BASELINE THAT MAY ONLY RISE. It is the one number that says
# how much of BC this tree can read, and it is measured over the WHOLE population rather than a
# sample -- 1 545 table objects in the BaseApp, every one of them, on every run of the lint.
if [ -x build/agirutc ] && [ -d "$AGIRU_BC_SOURCE" ]; then
  scan=$(build/agirutc "$AGIRU_BC_SOURCE" apps.json)
  tables=$(printf '%s' "$scan" | awk '/^tables/{print $2}')
  tableTotal=$(printf '%s' "$scan" | awk '/^tables/{print $4}')
  units=$(printf '%s' "$scan" | awk '/^codeunits/{print $2}')
  unitTotal=$(printf '%s' "$scan" | awk '/^codeunits/{print $4}')
  enums=$(printf '%s' "$scan" | awk '/^enums/{print $2}')
  enumTotal=$(printf '%s' "$scan" | awk '/^enums/{print $4}')
  pages=$(printf '%s' "$scan" | awk '/^pages/{print $2}')
  pageTotal=$(printf '%s' "$scan" | awk '/^pages/{print $4}')
  read -r allowedTables allowedTableTotal allowedUnits allowedUnitTotal allowedEnums \
    allowedEnumTotal allowedPages allowedPageTotal <<EOT
$(cat test/transpile-baseline 2>/dev/null || echo "0 0 0 0 0 0 0 0")
EOT
  ut=$(printf '%s' "$scan" | awk '/^UT /{print $4}')
  utReached=$(printf '%s' "$scan" | awk '/of them reach the parser/{print $1}')
  printf 'lint: %s of %s tables, %s of %s codeunits, %s of %s pages and %s of %s enums parse\n' \
    "$tables" "$tableTotal" "$units" "$unitTotal" "$pages" "$pageTotal" "$enums" "$enumTotal"
  # THE MILESTONE'"'"'S DENOMINATOR IS COUNTED FROM THE TEXT AND NEVER FROM THE PARSE, so that a
  # parser that loses a file cannot quietly shrink the population it is measured against.
  printf 'lint: %s of %s UT [Test] methods reach the parser\n' "$utReached" "$ut"
  printf 'lint: the baseline requires %s tables, %s codeunits, %s pages and %s enums\n' \
    "$allowedTables" "$allowedUnits" "$allowedPages" "$allowedEnums"
  if [ "$tables" -lt "$allowedTables" ] || [ "$units" -lt "$allowedUnits" ] ||
     [ "$pages" -lt "$allowedPages" ] || [ "$enums" -lt "$allowedEnums" ]; then
    printf 'lint: THE POPULATION FELL. A commit raises it or leaves it; it never lowers it.\n' >&2
    exit 1
  fi
  if [ "$tables" -gt "$allowedTables" ] || [ "$units" -gt "$allowedUnits" ] ||
     [ "$pages" -gt "$allowedPages" ] || [ "$enums" -gt "$allowedEnums" ] ||
     [ "$tableTotal" -ne "$allowedTableTotal" ] || [ "$unitTotal" -ne "$allowedUnitTotal" ] ||
     [ "$pageTotal" -ne "$allowedPageTotal" ] || [ "$enumTotal" -ne "$allowedEnumTotal" ]; then
    printf '%s %s %s %s %s %s %s %s\n' "$tables" "$tableTotal" "$units" "$unitTotal" "$enums" \
      "$enumTotal" "$pages" "$pageTotal" > test/transpile-baseline
    printf 'lint: baseline raised to "%s %s %s %s %s %s %s %s" -- commit it with the widening.\n' \
      "$tables" "$tableTotal" "$units" "$unitTotal" "$enums" "$enumTotal" "$pages" "$pageTotal"
  fi
else
  printf 'lint: agirutc and the AL source are required for FULL=1.\n' >&2
  exit 2
fi

printf '\n== the AL surface ==\n'
# HOW MUCH OF AL THIS RUNTIME CAN DO, over the WHOLE documented surface rather than a sample: 1 253
# methods across 93 types, read from methods-auto/ because the documentation is the specification
# and it is complete. The predecessor implements 1 174 of them and is 97 % green on the UT subset,
# which makes its set the ORDER to work in rather than the target.
#
# The count comes from doxygen's XML -- what the DOOR declares, since a method that generated code
# cannot reach is not implemented. It may only rise.
if [ -f doc/al-surface.json ] && [ -d build/doc/xml ]; then
  surface=$(python3 scripts/al_surface.py --count)
  read -r allowedSurface surfaceTotal <<EOT
$(cat test/surface-baseline 2>/dev/null || echo "0 0")
EOT
  printf 'lint: %s of %s documented AL methods reachable, the baseline requires %s\n' \
    "$surface" "$surfaceTotal" "$allowedSurface"
  if [ "$surface" -lt "$allowedSurface" ]; then
    printf 'lint: THE SURFACE SHRANK by %s. A commit widens it or leaves it; it never narrows it.\n' \
      "$((allowedSurface - surface))" >&2
    exit 1
  fi
  if [ "$surface" -gt "$allowedSurface" ]; then
    printf '%s %s\n' "$surface" "$surfaceTotal" > test/surface-baseline
    printf 'lint: surface baseline raised to %s -- commit it with the widening.\n' "$surface"
  fi
else
  printf 'lint: required AL surface input is missing.\n' >&2
  exit 2
fi
