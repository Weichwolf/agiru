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
# HOW HARD THE PATH-SENSITIVE ANALYSER LOOKS IS A BUDGET, AND THE DEFAULT IS NOT AFFORDABLE HERE.
# Measured 2026-09-03 on src/rt/Record.cpp, one file: 30 s at clang's default of 225 000 nodes,
# 16 s at 50 000, 14 s at 20 000 -- and 11 s with the analyser off altogether, so it is two thirds
# of the cost. Over 46 units on two cores that is 11.5 minutes against 6, and a gate that takes
# eleven minutes is a gate nobody runs before a commit.
#
# IT IS A BUDGET AND NOT AN EXCEPTION. The check still runs on every path it can reach inside it;
# what it gives up is the deepest ones. `DEEP=1` restores clang's own number for the run that wants
# it, and the baseline is the same either way -- which is the point: a finding the budget hides is
# a finding DEEP=1 still has to find before a commit lands.
NODES=${DEEP:+225000}
NODES=${NODES:-50000}
# `clang-tidy` takes `--extra-arg`, `run-clang-tidy` takes `-extra-arg`. One dash apart, and the
# wrong one made the wrapper refuse the whole run -- which the gate then counted as a finding
# rather than reporting a pass, which is the behaviour that caught it.
BUDGET="-extra-arg=-Xclang -extra-arg=-analyzer-config -extra-arg=-Xclang -extra-arg=max-nodes=$NODES"
ONEBUDGET="--extra-arg=-Xclang --extra-arg=-analyzer-config --extra-arg=-Xclang --extra-arg=max-nodes=$NODES"
# ONLY WHAT CHANGED, UNLESS THE WHOLE TREE IS ASKED FOR. clang-tidy costs 16 times what the
# compiler costs on the same file -- measured 2026-09-02 on src/rt/Table.cpp: 2.0 s to parse it,
# 32.6 s to check it, of which 20.8 s is the path-sensitive analyzer alone. Over 97 units that is
# minutes, and a check nobody runs finds nothing, so the default is the one that gets run.
#
# THE FAST RUN NEVER WRITES A BASELINE, and it says so on every line it prints. A baseline measured
# over a subset is a false floor -- CLAUDE.md names it as a trap -- and the whole point of the
# counters is that they cannot be lowered by looking at less.
if [ -z "$FULL" ]; then
  # `git diff` DOES NOT SEE A FILE THAT IS NOT TRACKED YET, so a brand-new source -- exactly the
  # kind most likely to carry a finding -- would never be checked. `git status --porcelain` lists
  # modified and untracked alike, which is what "changed" has to mean here.
  # AND A DELETED FILE IS NOT A UNIT OF ANALYSIS. `git status` lists it, clang-tidy cannot find it
  # in `compile_commands.json`, and the whole run dies on
  # `unable to handle compilation, expected exactly one compiler job in ''` -- which names neither
  # the file nor the reason. Moving one source to another directory was enough.
  ours=$(git status --porcelain -- 'src/*' 'include/*' 'test/*' 2>/dev/null |
    sed 's/^...//' | grep -E '\.(cpp|h)$' | grep -v '^test/target/' || true)
  # ONE FILE PER LINE AND NOT ONE LINE OF FILES. Joined with spaces, `printf '%s\n' "$ours"` prints
  # a SINGLE line, so `grep '.cpp$'` asks whether the LAST name ends in `.cpp` and answers for all
  # of them: with a `.cpp` last, every changed HEADER was handed to clang-tidy as though it were a
  # translation unit -- which is where "redefinition of 'FieldDef'" came from, a header parsed twice
  # outside any TU. With a `.h` last, the grep matched nothing and the run analysed NOTHING and
  # reported a pass. That is CLAUDE.md's blind gate, and it was live in both directions.
  kept=""
  for f in $ours; do
    [ -f "$f" ] && kept=$(printf '%s\n%s' "$kept" "$f")
  done
  ours=$kept
fi
# THE UNIT COUNT COMES FROM compile_commands.json, WHICH WILL ONE DAY CARRY apps/ TOO. The day
# AGIRU_BUILD_APPS defaults on, this number jumps by some thousands while the analysis still skips
# them -- so it is counted the same way it is analysed, and the two cannot drift apart.
# UNIQUE FILES, because a source compiled into two targets is one unit of analysis and not two.
# The count was 116 against 62 files once, and when the gates stopped recompiling the same sources
# it fell to the file count -- which read as a SHRINKING DENOMINATOR and aborted a run that had lost
# nothing. A number that moves when the build's shape changes cannot guard against a tree that lost
# a file.
units=$(grep '"file"' compile_commands.json | grep -v '/apps/' | sort -u | wc -l | tr -d ' ')
# test/target/ IS GENERATED CODE, written by hand only because the gate needs a fixed image to
# compare the generator against. It falls out of the analysis for the same reason apps/ does: a
# finding there has no address, since nobody edits the file -- the emitter is what would have to
# change, and the emitter is analysed. What holds it instead is the compiler, which builds it into
# every gate.
if [ -z "$FULL" ]; then
  : > "$REPORT/tidy.log"
  # A CHANGED HEADER IS NOT A UNIT OF ANALYSIS, it is checked through the sources that include it.
  # What the gate must not do is call that a pass when nothing was checked at all -- see below.
  for f in $(printf '%s\n' "$ours" | grep '\.cpp$' || true); do
    # shellcheck disable=SC2086
    "$TIDY" -p . --quiet $ONEBUDGET "$f" >> "$REPORT/tidy.log" 2>&1 || true
  done
elif [ -n "$RUNTIDY" ]; then
  # shellcheck disable=SC2086
  "$RUNTIDY" -p . -quiet -j "$(nproc)" $BUDGET '^(?!.*/(apps|test/target)/).*\.cpp$' > "$REPORT/tidy.log" 2>&1 || true
else
  : > "$REPORT/tidy.log"
  for f in $(echo "$ours" | grep '\.cpp$' | grep -v '/test/target/'); do
    # shellcheck disable=SC2086
    "$TIDY" -p . --quiet $ONEBUDGET "$f" >> "$REPORT/tidy.log" 2>&1 || true
  done
fi
grep 'warning:\|error:' "$REPORT/tidy.log" | sed 's/ \[/\t[/' | sort -u > "$REPORT/tidy.unique"
found=$(wc -l < "$REPORT/tidy.unique" | tr -d ' ')

if [ -z "$FULL" ]; then
  # AND A RUN THAT ANALYSED NOTHING IS NOT A PASS. Changed headers with no changed source is the
  # ordinary way to reach it, and it has to say so rather than print a zero that reads like green.
  checked=$(printf '%s\n' "$ours" | grep -c '\.cpp$' || true)
  changed=$(printf '%s\n' "$ours" | grep -c . || true)
  if [ "$checked" -eq 0 ] && [ "$changed" -gt 0 ]; then
    printf 'lint: %s changed file(s), NONE of them a translation unit -- nothing was analysed.\n' \
      "$changed"
    printf 'lint: a changed header is checked through a source that includes it. `FULL=1` does.\n'
  fi
  printf 'lint: %s finding(s) over %s changed file(s)\n' \
    "$found" "$(printf '%s\n' "$ours" | grep -c . || echo 0)"
  printf 'lint: THIS IS NOT THE BASELINE. `make lint FULL=1` reads the whole tree and writes it.\n'
  [ "$found" -eq 0 ] || { cat "$REPORT/tidy.unique" >&2; exit 1; }
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

if [ -z "$FULL" ]; then
  printf '\n== silent places ==\n'
  # THE SAME COUNTER THE FULL RUN USES, and not a second one that looks like it. Written twice, the
  # two disagreed on the first try -- 3 against 0 -- because the copy counted every `catch (...)`
  # including one that REPORTS, and counted the word inside the comment explaining the word. Two
  # counters that disagree are worse than one that is slow.
  printf 'lint: %s silent place(s) in the tree\n' "$(grep_silent | wc -l | tr -d ' ')"
  printf '\nlint: the door, the AL population and the AL surface need FULL=1.\n'
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
  doxygen doc/Doxyfile >/dev/null 2>&1 || true
  # ONE WARNING CLASS IS DOXYGEN'"'"'S OWN LIMIT AND NOT A MISSING SIGN. `Text<N> : Text<0>` and
  # `Code<N> : Code<0>` are legal C++ -- a primary template deriving from its own specialisation --
  # and doxygen reports each of them as a "potential recursive class relation". The construct is
  # what lets AL hand a `Text[30]` to a `var Text` parameter, so the finding argues with AL rather
  # than with us; every other warning still counts, including a name in the same file.
  grep -v "recursive class relation" build/doc/warnings.txt > build/doc/undocumented.txt 2>/dev/null
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
  printf 'lint: doxygen is not installed, so the door is not checked.\n' >&2
fi

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
  printf 'lint: agirutc or the AL source is missing, so the population is not measured.\n' >&2
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
  printf 'lint: the AL surface is not measured (doc/al-surface.json or build/doc/xml missing).\n' >&2
fi
