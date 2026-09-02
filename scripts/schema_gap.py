#!/usr/bin/env python3
"""How much of the CRONUS dataset the transpiled schema can hold, measured rather than assumed.

The demo database is 28.4 and the AL source is 30.0 (CLAUDE.md), and that gap has to surface
somewhere it can be judged. It surfaces here: every table CRONUS carries and every column in it,
against the tables the transpiler emitted and the fields they declare.

THE COMPARISON IS BY AL NAME AND NOT BY SQL NAME. BC's SQL naming replaces `. / %` with `_` and
keeps the length, so `No_` is `No.` and `Sell-to Customer No_` is `Sell-to Customer No.` -- one
underscore stands for several different characters and the map is therefore many-to-one. Comparing
the collapsed forms is what makes it a comparison at all.
"""
import collections
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SYSTEM = {"systemid", "systemcreatedat", "systemcreatedby", "systemmodifiedat",
          "systemmodifiedby", "systemrowversion"}
DSN = "postgresql://agiru:agiru@localhost:5433/cronus"


def sql(query):
    out = subprocess.run(["psql", DSN, "-tAF\x1f", "-c", query],
                         capture_output=True, text=True, check=True).stdout
    return [line.split("\x1f") for line in out.splitlines() if line]


def collapsed(name):
    """The comparable form: SQL's `_` stands for several characters, so all of them go."""
    return re.sub(r"[^a-z0-9]", "", name.lower())


def emitted():
    """Every generated table: AL name -> the AL names of its fields."""
    tables = {}
    for header in (ROOT / "apps").rglob("*/table/*.h"):
        text = header.read_text(errors="replace")
        name = re.search(r'kName\{"((?:[^"\\]|\\.)*)"\}', text)
        if not name:
            continue
        fields = re.findall(r'Declare<&[^>]+>\([^,]+,\s*"((?:[^"\\]|\\.)*)"', text)
        # THE FIVE SYSTEM FIELDS ARE APPENDED BY `WithSystemFields<T>` AT COMPILE TIME, so no
        # literal names them in the header. Their SQL columns are `$systemId` and the rest.
        tables[collapsed(name.group(1))] = (name.group(1),
                                            {collapsed(f) for f in fields} | SYSTEM)
    return tables


def main():
    rows = sql("""select table_schema, table_name, column_name
                  from information_schema.columns
                  where table_schema not in ('pg_catalog', 'information_schema')
                  order by 1, 2, 3""")
    columns = collections.defaultdict(set)
    for schema, table, column in rows:
        columns[(schema, table)].add(column)

    generated = emitted()
    missingTables = []
    missingColumns = collections.Counter()
    matched = 0
    coveredColumns = 0
    totalColumns = 0
    for (schema, table), names in sorted(columns.items()):
        # `$ndo$...` IS THE PLATFORM'S OWN STORAGE and not an AL object at all -- the tenant
        # database's bookkeeping, which no `.al` file declares and the transpiler will never emit.
        if table.lower().startswith("$ndo$"):
            continue
        found = generated.get(collapsed(table))
        if found is None:
            missingTables.append(table)
            continue
        matched += 1
        alName, fields = found
        for column in names:
            # `timestamp` is the SQL rowversion and carries no AL field number (board:0013).
            if column == "timestamp":
                continue
            totalColumns += 1
            if collapsed(column) in fields:
                coveredColumns += 1
            else:
                missingColumns[f"{alName}.{column}"] += 1

    distinct = {t for t in missingTables}
    print(f"cronus    {len(columns)} relations, {len(distinct)} of them with no generated table")
    print(f"matched   {matched} relations against a generated table")
    print(f"columns   {coveredColumns} of {totalColumns} land on a declared field "
          f"({100.0 * coveredColumns / totalColumns:.2f} %)")
    print()
    print("the columns with no field, most frequent first")
    for name, count in missingColumns.most_common(20):
        print(f"  {count:>4}  {name}")
    print()
    print("the tables with no generated counterpart, first 20 of "
          f"{len(distinct)}")
    for name in sorted(distinct)[:20]:
        print(f"        {name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
