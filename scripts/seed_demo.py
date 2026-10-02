#!/usr/bin/env python3
"""Seed the runner's database with the CRONUS demo data.

WHY. The runner makes its own schema -- `ProvisionInstalled` creates the 1 602 tables the binary
carries -- and fills none of it, so every test starts against an empty company. BC's own run does not work like that:
`Run-TestsInBcContainer` runs against a CRONUS container, and the BaseApp's test libraries assume
the demo company is there. Measured 2026-09-08 over the whole population, about 7 000 of the 19 543
failures are five sentences of the form `The Sales & Receivables Setup does not exist` -- the
runtime answering correctly about an empty table (board:0613).

WHAT IT COPIES. The rows of every table that exists in BOTH, column by column, for the columns that
exist in both. The demo database is 28.4 and the transpiled schema is 30.0, so:

  * a table only 30.0 has stays empty -- there is nothing to put in it;
  * a column only 30.0 has keeps its default -- 28.4 never wrote one;
  * a column only 28.4 has is dropped -- 30.0 has nowhere to put it.

Each of those three is COUNTED and printed, because a silent one is the difference between a
dataset that is 28.4's and one that is a subset nobody measured (board:0004).

A TABLE THAT REFUSES DOES NOT STOP THE OTHERS, and its reason is printed with its name. A run that
seeds NOTHING is an abort and not a pass.

    python3 scripts/seed_demo.py [--into agiru_test_0] [--company "CRONUS International Ltd"]
"""

import argparse
import shlex
import subprocess
import sys

CTR = "agiru-pg"
RUNNER = "agiru_test_0"
SOURCE = "cronus"


def fail(message):
    print(f"seed: {message}", file=sys.stderr)
    raise SystemExit(1)


def psql(database, sql, quiet=True):
    argv = ["podman", "exec", CTR, "psql", "-U", "agiru", "-d", database,
            "-tAc" if quiet else "-c", sql]
    done = subprocess.run(argv, capture_output=True, check=False)
    if done.returncode != 0:
        return None, done.stderr.decode("utf-8", "replace").strip()
    return done.stdout.decode("utf-8", "replace"), ""


TYPES = {}


def columns(database, schema):
    out, why = psql(database, (
        "select table_name || '\t' || column_name || '\t' || data_type from "
        f"information_schema.columns where table_schema = '{schema}' "
        "order by table_name, ordinal_position"))
    if out is None:
        fail(f"{database} refused its column list: {why}")
    held = {}
    for line in out.splitlines():
        if line.count("\t") < 2:
            continue
        table, column, kind = line.split("\t", 2)
        held.setdefault(table, []).append(column)
        TYPES[(database, table, column)] = kind
    return held


# A NULL IN 28.4 LANDS AS THE COLUMN'S BLANK. The transpiled schema declares every column NOT NULL
# -- AL has no null, a blank Text is '' and an empty Blob is empty -- while SQL Server's demo rows
# hold NULL in a blob or text nobody wrote (`Sales Header."Work Description"`). Eighteen tables
# refused for it, `Sales Header` among them (measured 2026-09-08).
# Dollar-quoted, because the SELECT travels inside a single-quoted `psql -c` inside `sh -c`.
BLANKS = {"bytea": "$$$$::bytea", "text": "$$$$", "character varying": "$$$$", "character": "$$$$"}


def blanked(database, table, column, into_kind, alias=""):
    blank = BLANKS.get(into_kind)
    quoted_name = alias + '"' + column.replace('"', '""') + '"'
    return f"COALESCE({quoted_name}, {blank})" if blank else quoted_name


def quoted(names):
    return ", ".join('"' + name.replace('"', '""') + '"' for name in names)


# SQL SERVER'S BC SCHEMA SPELLS A DOT AS AN UNDERSCORE, and the system fields with a `$`.
# `Assembly Order Nos.` is `Assembly Order Nos_` there and `SystemId` is `$systemId`; the transfer
# carried the names across ONE TO ONE, which is what makes `cronus` a reference rather than a
# product of this tree. So the fold happens here, on the way in, and it happens on the AGIRU side:
# every transpiled name is folded to what SQL Server would have called it, and the demo column is
# looked up in that.
SYSTEM = {"$systemid": "SystemId", "$systemcreatedat": "SystemCreatedAt",
          "$systemcreatedby": "SystemCreatedBy", "$systemmodifiedat": "SystemModifiedAt",
          "$systemmodifiedby": "SystemModifiedBy"}


def folded(name):
    """What SQL Server would have called this transpiled name.

    BC folds every character SQL Server will not take in an identifier to an underscore: the dot
    of `Nos.`, the slash of `G/L Entry`, a quote, a bracket, a percent. `G/L Account` and the
    whole general ledger were skipped as "only 28.4 has" while the fold knew only the dot
    (measured 2026-09-08: 586 tables unmatched, `G/L Entry` among them).
    """
    for unsafe in './\\"\'%[]:':
        name = name.replace(unsafe, "_")
    return name


def matched(theirs, ours):
    """Their names against ours, as pairs; a fold that is not one-to-one is REFUSED rather than
    guessed at (CLAUDE.md: a mechanical pass cannot tell a naming defect from a gap)."""
    by_fold = {}
    ambiguous = set()
    for name in ours:
        key = folded(name)
        if key in by_fold:
            ambiguous.add(key)
        by_fold[key] = name
    pairs = []
    for name in theirs:
        ours_name = SYSTEM.get(name.lower())
        if ours_name is None:
            ours_name = by_fold.get(name)
        if ours_name is None or folded(ours_name) in ambiguous or ours_name not in ours:
            continue
        pairs.append((name, ours_name))
    return pairs


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--company", default="CRONUS International Ltd")
    parser.add_argument("--into", default=RUNNER)
    arguments = parser.parse_args()

    source = columns(SOURCE, arguments.company)
    if not source:
        fail(f"{SOURCE}.\"{arguments.company}\" has no columns at all")

    target = columns(arguments.into, "public")
    if not target:
        fail(f"{arguments.into}.public has no columns at all. The schema is made by the runner's "
             f"own ProvisionInstalled, so run `agiru run-tests --codeunit <any>` once first.")
    by_fold = {folded(name): name for name in target}
    shared = sorted((theirs, by_fold[theirs]) for theirs in source if theirs in by_fold)
    if not shared:
        fail("no table exists in both. ABORT, not a pass.")

    seeded = 0
    rows = 0
    empty = 0
    refused = []
    dropped = 0
    defaulted = 0
    for theirs, ours in shared:
        # A TABLE'S COMPANION CARRIES ITS OTHER HALF. 28.4 stores the fields an app adds to another
        # app's table in `<Table>$ext`, keyed like the table -- and after the BaseApp split that is
        # where `Source Code Setup."General Journal"` lives (148 companions, 1 329 columns, measured
        # 2026-09-09). Without the join every such column seeded as blank, and the test library's
        # `FindGeneralJournalSourceCode` found no template (board:0635).
        companion = theirs + "$ext"
        base_columns = source[theirs]
        extra = [c for c in source.get(companion, []) if c not in base_columns]
        pairs = matched(base_columns + extra, target[ours])
        dropped += len(base_columns) + len(extra) - len(pairs)
        defaulted += len(target[ours]) - len(pairs)
        if not pairs:
            continue
        table = ours
        selected = ", ".join(
            blanked(SOURCE, theirs, p[0], TYPES.get((arguments.into, ours, p[1]), ""),
                    "e." if p[0] in extra else "b.")
            for p in pairs)
        joined = ""
        if any(p[0] in extra for p in pairs):
            key = [c for c in base_columns
                   if c in source[companion] and c != "timestamp" and not c.startswith("$")]
            if not key:
                refused.append((table, "the companion shares no key column"))
                continue
            joined = (f' LEFT JOIN "{arguments.company}"."{companion}" e ON ' +
                      " AND ".join(f'b."{c}" = e."{c}"' for c in key))
        reading = ('\\copy (SELECT ' + selected +
                   f' FROM "{arguments.company}"."{theirs}" b{joined}) TO STDOUT')
        writing = '\\copy public."' + ours + '" (' + quoted(p[1] for p in pairs) + ') FROM STDIN'
        # A COLUMN MAY CARRY AN APOSTROPHE (`Relative's Employee No.`), and the command runs
        # through `sh -c`: the statements are quoted the way the shell wants them.
        piped = (f"psql -U agiru -d {SOURCE} -c {shlex.quote(reading)} | "
                 f"psql -U agiru -d {arguments.into} -c {shlex.quote(writing)}")
        done = subprocess.run(["podman", "exec", CTR, "sh", "-c", piped],
                              capture_output=True, check=False)
        note = done.stdout.decode("utf-8", "replace") + done.stderr.decode("utf-8", "replace")
        if done.returncode != 0:
            first = [l for l in note.splitlines() if l.startswith("ERROR:")]
            refused.append((table, first[0] if first else note.strip()[:120]))
            continue
        copied = 0
        for line in note.splitlines():
            if line.startswith("COPY "):
                copied = max(copied, int(line.split()[1]))
        rows += copied
        if copied == 0:
            empty += 1
            continue
        seeded += 1

    print(f"seed: {seeded} table(s) carry {rows} row(s); {empty} were empty in the demo data")
    print(f"seed: {len(source) - len(shared)} table(s) only 28.4 has, "
          f"{len(target) - len(shared)} only the transpiled schema has")
    print(f"seed: {dropped} column(s) 28.4 has and the schema does not, "
          f"{defaulted} the schema has and 28.4 does not")
    if refused:
        print(f"seed: {len(refused)} table(s) refused:")
        for table, why in refused[:20]:
            print(f"        {table}: {why}")
    if rows == 0:
        fail("not one row went in. ABORT, not a pass.")


if __name__ == "__main__":
    main()
