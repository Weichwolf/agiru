#!/usr/bin/env python3
"""Copy the restored CRONUS demo database from SQL Server into PostgreSQL, one to one.

It knows nothing about AL. Names, types and rows are carried across as SQL Server holds them, so
the result is BC's own schema and BC's own data -- a REFERENCE to measure agiru's schema against
rather than a product of it. That is what makes it independent of the transpiler: it runs before a
single generated table compiles.

The rows travel in PostgreSQL's COPY TEXT format, and the escaping happens in SQL Server: each row
is SELECTed as one already-escaped string, so nothing between the two databases has to parse or
re-quote a value. `bcp` writes those strings without padding or wrapping, which `sqlcmd` cannot
promise, and `psql` reads them with COPY.
"""
import argparse
import re
import subprocess
import sys

MSSQL = "agiru-mssql"
PG = "agiru-pg"
SQLCMD = "/opt/mssql-tools18/bin/sqlcmd"
BCP = "/opt/mssql-tools18/bin/bcp"

# SQL Server type -> PostgreSQL type. `precision` and `scale` fill the braces.
TYPES = {
    "bit": "boolean",
    "tinyint": "smallint",
    "smallint": "smallint",
    "int": "integer",
    "bigint": "bigint",
    "decimal": "numeric({p},{s})",
    "numeric": "numeric({p},{s})",
    "money": "numeric(19,4)",
    "smallmoney": "numeric(10,4)",
    "float": "double precision",
    "real": "real",
    "date": "date",
    "time": "time",
    "datetime": "timestamp",
    "datetime2": "timestamp",
    "smalldatetime": "timestamp",
    "datetimeoffset": "timestamptz",
    "uniqueidentifier": "uuid",
    "char": "varchar({n})",
    "nchar": "varchar({n})",
    "varchar": "varchar({n})",
    "nvarchar": "varchar({n})",
    "text": "text",
    "ntext": "text",
    "xml": "text",
    "binary": "bytea",
    "varbinary": "bytea",
    "image": "bytea",
    "timestamp": "bytea",
    "rowversion": "bytea",
    "sql_variant": "text",
}

BINARY = {"binary", "varbinary", "image", "timestamp", "rowversion"}


def run(cmd, stdin=None, capture=True):
    return subprocess.run(cmd, input=stdin, capture_output=capture, text=True, check=False)


def mssql(query, password, database="CRONUS"):
    out = run(["podman", "exec", MSSQL, SQLCMD, "-S", "localhost", "-U", "sa", "-P", password,
               "-C", "-d", database, "-h-1", "-W", "-s", "\x1f", "-Q", "SET NOCOUNT ON; " + query])
    if out.returncode != 0:
        sys.exit(f"cronus: sqlcmd refused: {out.stderr.strip() or out.stdout.strip()}")
    return [line.split("\x1f") for line in out.stdout.splitlines() if line.strip()]


def psql(statement, database):
    # ON STDIN AND NOT ON THE COMMAND LINE. The whole schema is one script of 2 129 statements, and
    # `execve` refuses an argument that long with `Argument list too long` -- which arrives as a
    # Python traceback rather than as anything a reader would connect to SQL.
    out = run(["podman", "exec", "-i", PG, "psql", "-U", "agiru", "-d", database, "-v",
               "ON_ERROR_STOP=1", "-q", "-f", "-"], stdin=statement)
    if out.returncode != 0:
        sys.exit(f"cronus: psql refused: {out.stderr.strip()}")
    return out.stdout


def pg_type(name, length, precision, scale):
    shape = TYPES.get(name)
    if shape is None:
        sys.exit(f"cronus: no PostgreSQL type for SQL Server '{name}'")
    chars = length // 2 if name.startswith("n") and length > 0 else length
    if length < 0 or chars > 10485760:
        shape = "text" if "varchar" in shape else shape
    return shape.format(p=precision or 38, s=scale or 0, n=max(chars, 1))


def escaped(column, name):
    """One column as PostgreSQL COPY TEXT: NULL is \\N, and the four escapes COPY reads."""
    quoted = f"[{column}]"
    if name in BINARY:
        # COPY TEXT READS `\\xNN` AS A BYTE ITSELF, so bytea's own `\\x` prefix has to arrive
        # escaped or the stream says NUL where it means "hex follows".
        value = f"'\\\\x' + CONVERT(varchar(max), CAST({quoted} AS varbinary(max)), 2)"
    elif name == "bit":
        value = f"CASE WHEN {quoted} = 1 THEN 't' ELSE 'f' END"
    elif name in ("datetime", "datetime2", "smalldatetime", "datetimeoffset"):
        value = f"CONVERT(varchar(33), {quoted}, 126)"
    elif name in ("float", "real"):
        value = f"CONVERT(varchar(64), {quoted}, 3)"
    else:
        # BC MIXES COLLATIONS INSIDE ONE TABLE -- `Latin1_General_100_CS_AS` beside `CI_AS` -- and
        # `+` between two of them is ambiguous rather than merely odd. One collation for every
        # string in the row settles it without touching a value.
        value = f"CAST({quoted} AS nvarchar(max)) COLLATE DATABASE_DEFAULT"
        value = (f"REPLACE(REPLACE(REPLACE(REPLACE({value}, CHAR(92), CHAR(92)+CHAR(92)), "
                 f"CHAR(9), CHAR(92)+'t'), CHAR(10), CHAR(92)+'n'), CHAR(13), CHAR(92)+'r')")
    return f"CASE WHEN {quoted} IS NULL THEN CHAR(92)+'N' ELSE {value} END"


GUID = re.compile(r"\$[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}"
                  r"(\$ext)?$")


def bare(name):
    """The name without the app GUID, keeping a `$ext` suffix when one follows it.

    BC appends the extending app's GUID to a tableextension's storage and to its columns:
    `Default Trans_ Type - Return$70912191-3c4c-49fc-a1de-bc6ea1ac9da6` is 65 bytes,
    `Assisted Company Setup Status$437dbf0e-...$ext` is 70, and PostgreSQL cuts at 63. The GUID says
    which app added the thing and nothing about the value, so it goes -- and the collision that
    dropping it could cause is DETECTED rather than hoped away.
    """
    found = GUID.search(name)
    return name[:found.start()] + (found.group(1) or "") if found else name


def placed(table):
    """Where a SQL Server table lands: a schema per company, the AL table name as the table.

    PostgreSQL cuts an identifier at 63 bytes and says so in a NOTICE, which is a silent collision
    waiting for two tables whose first 63 characters agree. BC's own names run to 97 --
    `CRONUS International Ltd_$Item Charge Assignment (Purch)$437dbf0e-...$ext` -- so a one-to-one
    copy of the NAME is not possible. Splitting it loses nothing: the company becomes the schema and
    the AL table name becomes the table. Measured over the restored 28.4 demo database: 1 864 tables
    carry a company, 44 are `$ndo$` platform tables, 221 are tenant-scoped or plain.
    """
    if table.startswith("$ndo$"):
        return "platform", table
    cut = table.find("_$")
    if cut < 0:
        return "system", bare(table)
    return table[:cut], bare(table[cut + 2:])


def naming(tables):
    """Every table's schema and name, with the app GUID kept exactly where it distinguishes.

    Dropping it is right 2 127 times out of 2 129 and WRONG twice: two apps store
    `CRONUS International Ltd_$Dimension Set Entry$...` under different GUIDs, so the bare name
    would name both. The GUID is therefore not redundant, it is redundant USUALLY -- so the ones
    that collide keep the first eight hex digits of theirs and the rest stay short.
    """
    bared = {t: placed(t) for t in tables}
    seen = {}
    for table, where in bared.items():
        seen.setdefault(where, []).append(table)
    for where, sharing in seen.items():
        if len(sharing) == 1:
            continue
        for table in sharing:
            found = GUID.search(table)
            if found is None:
                sys.exit(f'cronus: "{table}" collides on {where} and carries no GUID to tell it '
                         f'apart from {sorted(set(sharing) - {table})}')
            schema, name = where
            bared[table] = (schema, f"{name}${table[found.start() + 1:found.start() + 9]}")
    return bared


def fits(name, what):
    if len(name.encode()) > 63:
        sys.exit(f"cronus: the {what} \"{name}\" is {len(name.encode())} bytes and PostgreSQL "
                 f"cuts at 63 -- it would collide silently")
    return name


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--password", default="Agiru!Pass2026")
    ap.add_argument("--database", default="cronus")
    ap.add_argument("--only", help="copy just the tables whose name contains this")
    ap.add_argument("--schema-only", action="store_true")
    args = ap.parse_args()

    tables = [row[0] for row in mssql(
        "SELECT name FROM sys.tables WHERE is_ms_shipped = 0 ORDER BY name", args.password)]
    # EVERY COLUMN IN ONE QUERY. Asking per table is 2 129 round trips through `podman exec`, which
    # costs more than the copying does.
    every = {}
    for row in mssql(
            "SELECT tb.name, c.name, t.name, c.max_length, c.precision, c.scale, c.is_nullable "
            "FROM sys.columns c JOIN sys.tables tb ON tb.object_id = c.object_id "
            "JOIN sys.types t ON t.user_type_id = c.user_type_id "
            "WHERE tb.is_ms_shipped = 0 ORDER BY tb.name, c.column_id", args.password):
        every.setdefault(row[0], []).append(row[1:])
    # TWO THIRDS OF THE TABLES ARE EMPTY -- 1 394 of 2 129 in the 28.4 demo database, measured --
    # and copying nothing still costs three container round trips. The counts come from the
    # catalogue in one query, so an empty table costs its CREATE and nothing else.
    held = {row[0]: int(row[1]) for row in mssql(
        "SELECT t.name, SUM(p.rows) FROM sys.tables t "
        "JOIN sys.partitions p ON p.object_id = t.object_id AND p.index_id IN (0, 1) "
        "WHERE t.is_ms_shipped = 0 GROUP BY t.name", args.password)}
    if args.only:
        tables = [t for t in tables if args.only.lower() in t.lower()]
    print(f"cronus: {len(tables)} table(s) to carry across")

    psql(f'DROP DATABASE IF EXISTS "{args.database}"', "postgres")
    psql(f'CREATE DATABASE "{args.database}"', "postgres")

    rows = 0
    made = set()
    where = naming(tables)
    statements = []
    carrying = []
    for i, table in enumerate(tables, 1):
        columns = every.get(table, [])
        if not columns:
            continue
        schema, name = where[table]
        fits(schema, "schema")
        fits(name, "table")
        named = [bare(c[0]) for c in columns]
        for column in named:
            fits(column, "column")
        if len(set(named)) != len(named):
            twice = sorted({c for c in named if named.count(c) > 1})
            sys.exit(f'cronus: dropping the app GUID makes "{table}" carry {twice} twice')
        ddl = ", ".join(
            f'"{n}" {pg_type(c[1], int(c[2]), int(c[3]), int(c[4]))}'
            for n, c in zip(named, columns))
        if schema not in made:
            statements.append(f'CREATE SCHEMA IF NOT EXISTS "{schema}"')
            made.add(schema)
        statements.append(f'CREATE UNLOGGED TABLE "{schema}"."{name}" ({ddl})')
        if held.get(table, 0) > 0:
            carrying.append((table, schema, name, columns))

    print(f"cronus: {len(statements)} statement(s) of schema, "
          f"{len(carrying)} table(s) with rows", flush=True)
    psql(";\n".join(statements), args.database)
    if args.schema_only:
        return

    refused = []
    for i, (table, schema, name, columns) in enumerate(carrying, 1):
        carried, why = copy_rows(table, schema, name, columns, args)
        if carried is None:
            refused.append((f'"{schema}"."{name}"', why))
        else:
            rows += carried
        if i % 100 == 0:
            print(f"cronus: {i}/{len(carrying)} tables, {rows} rows", flush=True)
    for what, why in refused:
        print(f"cronus: {what} REFUSED -- {why}", file=sys.stderr)
    if refused:
        print(f"cronus: {len(refused)} of {len(carrying)} tables with rows could not be carried",
              file=sys.stderr)
    print(f"cronus: {len(tables)} tables, {rows} rows in \"{args.database}\"")


def copy_rows(table, schema, name, columns, args):
    picked = " + CHAR(9) + ".join(escaped(c[0], c[1]) for c in columns)
    query = f'SELECT {picked} FROM [{table}]'
    out = run(["podman", "exec", MSSQL, BCP, query, "queryout", "/tmp/cronus.dat",
               "-S", "localhost", "-U", "sa", "-P", args.password, "-d", "CRONUS",
               "-c", "-C", "65001", "-r", "\n", "-u"])
    if out.returncode != 0:
        sys.exit(f"cronus: bcp refused for {table}: {out.stderr.strip() or out.stdout.strip()}")
    read = subprocess.Popen(["podman", "exec", MSSQL, "cat", "/tmp/cronus.dat"],
                            stdout=subprocess.PIPE)
    write = subprocess.Popen(
        ["podman", "exec", "-i", PG, "psql", "-U", "agiru", "-d", args.database,
         "-v", "ON_ERROR_STOP=1", "-q", "-c", f'COPY "{schema}"."{name}" FROM STDIN'],
        stdin=read.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    read.stdout.close()
    _, err = write.communicate()
    if write.returncode != 0:
        # ONE TABLE THAT CANNOT BE CARRIED MUST NOT STOP THE OTHER 734, AND MUST NOT VANISH EITHER.
        # `NAV App Installed App` keeps a hash in an `nvarchar` column: the value is not text, it
        # holds an unpaired surrogate and a U+0000, and PostgreSQL cannot store U+0000 in `text` at
        # all. No transfer format fixes that -- the column would have to become `bytea`, which is no
        # longer a copy of the schema. So it is reported and counted.
        return None, err.strip().splitlines()[0]
    return int([line for line in out.stdout.splitlines()
                if "rows copied" in line][0].split()[0]), None


if __name__ == "__main__":
    main()
