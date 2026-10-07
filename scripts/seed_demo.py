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
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import uuid
from typing import NamedTuple

CTR = "agiru-pg"
RUNNER = "agiru_test_0"
SOURCE = "cronus"


class Endpoint(NamedTuple):
    container: str = CTR
    user: str = ""


def psql_command(database, endpoint=None, stdin=False):
    endpoint = endpoint or Endpoint()
    argv = ["podman", "exec"]
    if stdin:
        argv.append("-i")
    if endpoint.user:
        argv.extend(["--user", endpoint.user])
    return argv + [endpoint.container, "psql", "-X", "-U", "agiru", "-v",
                   "ON_ERROR_STOP=1", "-d", database]


def fail(message):
    print(f"seed: {message}", file=sys.stderr)
    raise SystemExit(1)


def psql(database, sql, quiet=True, endpoint=None):
    argv = psql_command(database, endpoint, stdin=True) + (["-tA"] if quiet else [])
    done = subprocess.run(argv, input=(sql + '\n').encode(), capture_output=True, check=False)
    if done.returncode != 0:
        return None, done.stderr.decode("utf-8", "replace").strip()
    return done.stdout.decode("utf-8", "replace"), ""


TYPES = {}


def columns(database, schema, endpoint=None):
    out, why = psql(database, (
        "select table_name || '\t' || column_name || '\t' || data_type from "
        f"information_schema.columns where table_schema = {sql_literal(schema)} "
        "order by table_name, ordinal_position"), endpoint=endpoint)
    if out is None:
        fail(f"{database} refused its column list: {why}")
    held = {}
    for line in out.splitlines():
        if line.count("\t") < 2:
            continue
        table, column, kind = line.split("\t", 2)
        held.setdefault(table, []).append(column)
        TYPES[(endpoint or Endpoint(), database, table, column)] = kind
    return held


def column_kind(database, table, column, endpoint=None):
    return TYPES.get((endpoint or Endpoint(), database, table, column))


# A NULL IN 28.4 LANDS AS THE COLUMN'S BLANK. The transpiled schema declares every column NOT NULL
# -- AL has no null, a blank Text is '' and an empty Blob is empty -- while SQL Server's demo rows
# hold NULL in a blob or text nobody wrote (`Sales Header."Work Description"`). Eighteen tables
# refused for it, `Sales Header` among them (measured 2026-09-08).
# Dollar-quoted empty values keep the generated SELECT independent of text-field quoting.
BLANKS = {"bytea": "$$$$::bytea", "text": "$$$$", "character varying": "$$$$", "character": "$$$$"}


def blanked(database, table, column, into_kind, alias="", endpoint=None):
    blank = BLANKS.get(into_kind)
    quoted_name = alias + '"' + column.replace('"', '""') + '"'
    if (column == "timestamp" and into_kind == "bigint"
            and column_kind(database, table, column, endpoint) == "bytea"):
        return (f"CASE WHEN octet_length({quoted_name}) = 8 THEN "
                f"CASE WHEN get_byte({quoted_name}, 0) < 128 THEN "
                f"(('x' || encode({quoted_name}, 'hex'))::bit(64)::bigint)::text "
                f"ELSE {quoted_name}::text END ELSE {quoted_name}::text END")
    return f"COALESCE({quoted_name}, {blank})" if blank else quoted_name


def quoted(names):
    return ", ".join('"' + name.replace('"', '""') + '"' for name in names)


def digest(path):
    hashed = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            hashed.update(block)
    return hashed.hexdigest()


def sql_literal(value):
    return "'" + value.replace("'", "''") + "'"


def provenance(company, source, target, target_database, source_database=SOURCE,
               source_endpoint=None, target_endpoint=None):
    root = Path(__file__).resolve().parents[1]
    version = (root / 'BC_VERSION').read_text().strip()
    country = os.environ.get('BC_COUNTRY', 'w1')
    archive = root / 'work' / f'bc-{version}-{country}.zip'
    if not archive.is_file():
        fail(f"the BC artefact {archive} is missing; the seed cannot identify its source")
    bc_source = Path(os.environ.get('AGIRU_BC_SOURCE', Path.home() / 'Git/BCApps/src'))
    revision = subprocess.run(['git', '-C', str(bc_source), 'rev-parse', '--show-toplevel', 'HEAD'],
                              capture_output=True, text=True, check=False)
    identity = revision.stdout.splitlines()
    if (revision.returncode != 0 or len(identity) != 2 or not identity[1].strip()
            or Path(identity[0]).resolve() == root):
        fail(f"cannot identify BCApps revision under {bc_source}")
    def shape(tables, database, endpoint):
        return sorted((table, [(column, column_kind(database, table, column, endpoint))
                               for column in columns]) for table, columns in tables.items())
    return {
        'schema': 1,
        'id': str(uuid.uuid4()),
        'artefact_version': version,
        'artefact_sha256': digest(archive),
        'bc_source_revision': identity[1],
        'scope_sha256': digest(root / 'scope.json'),
        'source_schema_sha256': hashlib.sha256(
            json.dumps(shape(source, source_database, source_endpoint), sort_keys=True).encode()).hexdigest(),
        'target_schema_sha256': hashlib.sha256(
            json.dumps(shape(target, target_database, target_endpoint), sort_keys=True).encode()).hexdigest(),
        'company': company,
        'source_container': (source_endpoint or Endpoint()).container,
        'source_database': source_database,
        'target_container': (target_endpoint or Endpoint()).container,
        'target_database': target_database,
    }


def begin_seed(database, details, endpoint=None):
    stored = sql_literal(json.dumps(details, sort_keys=True))
    sql = ("BEGIN; CREATE TABLE IF NOT EXISTS public.agiru_seed_provenance "
           "(singleton boolean PRIMARY KEY DEFAULT true CHECK (singleton), "
           "status text NOT NULL, details jsonb NOT NULL); "
           f"INSERT INTO public.agiru_seed_provenance VALUES (true, 'building', {stored}::jsonb); "
           "COMMIT")
    result, why = psql(database, sql, endpoint=endpoint)
    if result is None:
        fail(f"{database} already has seed provenance or cannot record it: {why}")


def finish_seed(database, details, endpoint=None, status='complete'):
    stored = sql_literal(json.dumps(details, sort_keys=True))
    sql = (f"UPDATE public.agiru_seed_provenance SET status = {sql_literal(status)}, "
           f"details = {stored}::jsonb WHERE singleton AND status IN ('building', 'failed') "
           f"AND details->>'id' = {sql_literal(details['id'])} "
           "RETURNING details->>'id'")
    result, why = psql(database, sql, endpoint=endpoint)
    if result is None or details['id'] not in result.splitlines():
        fail(f"{database} could not mark its seed {status}: {why or result}")


def transfer(reading, writing, into, source_database=SOURCE,
             source_endpoint=None, target_endpoint=None):
    with tempfile.TemporaryFile() as source_errors:
        source = subprocess.Popen(
            psql_command(source_database, source_endpoint) + ["-c", reading],
            stdout=subprocess.PIPE, stderr=source_errors)
        try:
            target = subprocess.Popen(
                psql_command(into, target_endpoint, stdin=True) + ["-c", writing],
                stdin=source.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except OSError:
            source.terminate()
            source.wait()
            raise
        source.stdout.close()
        target_output, target_errors = target.communicate()
        source_status = source.wait()
        source_errors.seek(0)
        note = (target_output + target_errors + source_errors.read()).decode("utf-8", "replace")
        return source_status, target.returncode, note


def reconcile_rowversions(database, target, endpoint=None):
    tables = [name for name in target
              if column_kind(database, name, 'timestamp', endpoint) == 'bigint']
    if not tables:
        return
    maxima = ' UNION ALL '.join(
        f'SELECT COALESCE(MAX("timestamp"), 0) AS value FROM public.{quoted([name])}'
        for name in tables)
    sql = ("BEGIN; SELECT pg_catalog.pg_advisory_xact_lock(x'41475256'::integer, 1); "
           "SELECT pg_catalog.setval('agiru_platform.rowversions_v1'::regclass, "
           "GREATEST(agiru_platform.last_rowversion_v1(), value), true) "
           f"FROM (SELECT MAX(value) AS value FROM ({maxima}) AS tables) AS seed "
           "WHERE value > 0; COMMIT")
    result, why = psql(database, sql, endpoint=endpoint)
    if result is None:
        fail(f"{database} cannot advance its imported rowversions: {why}")


def canonical(expression, kind):
    if kind in ('smallint', 'integer', 'bigint', 'numeric', 'decimal'):
        return f'trim_scale(({expression})::numeric)'
    if kind == 'boolean':
        return f'({expression})::text::boolean'
    if kind == 'time without time zone':
        return f'({expression})::time::text'
    return f'({expression})::text'


def verify_rows(reading, writing, source_database, into, source_endpoint, target_endpoint):
    with tempfile.TemporaryDirectory(prefix='agiru-seed-readback-') as folder:
        files = [Path(folder) / name for name in ('source', 'target')]
        for path, database, endpoint, statement in zip(
                files, (source_database, into), (source_endpoint, target_endpoint),
                (reading, writing)):
            with path.open('wb') as output:
                result = subprocess.run(psql_command(database, endpoint, stdin=True),
                                        input=(statement + '\n').encode(), stdout=output,
                                        stderr=subprocess.PIPE, check=False)
            if result.returncode != 0:
                return 1, 1, result.stderr.decode('utf-8', 'replace')
            ordered = subprocess.run(['sort', '-o', str(path), str(path)],
                                     env={**os.environ, 'LC_ALL': 'C'},
                                     capture_output=True, check=False)
            if ordered.returncode != 0:
                return 1, 1, ordered.stderr.decode('utf-8', 'replace')
        if digest(files[0]) != digest(files[1]):
            return 1, 1, 'ERROR: typed source/target rows differ'
        with files[0].open('rb') as rows:
            count = sum(1 for _ in rows)
        return 0, 0, f'COPY {count}\n'


def resume_seed(database, details, endpoint):
    result, why = psql(database, "SELECT status || E'\\t' || details::text "
                       "FROM public.agiru_seed_provenance WHERE singleton", endpoint=endpoint)
    if result is None:
        fail(f"{database} cannot read interrupted seed identity: {why}")
    state, separator, stored = result.strip().partition('\t')
    if not separator or state not in ('building', 'failed'):
        fail(f"{database} is not an interrupted or failed seed")
    previous = json.loads(stored)
    if any(previous.get(key) != value for key, value in details.items() if key != 'id'):
        fail(f"{database} seed identity changed; refusing recovery")
    details['id'] = previous['id']


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
    guessed at (board:0004: a mechanical pass cannot tell a naming defect from a gap)."""
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
    parser.add_argument("--source-database", default=SOURCE)
    parser.add_argument("--source-container", default=CTR)
    parser.add_argument("--target-container", default=CTR)
    parser.add_argument("--source-exec-user", default="")
    parser.add_argument("--target-exec-user", default="")
    parser.add_argument("--verify", action="store_true",
                        help="read back every shared row before finalizing an interrupted seed")
    arguments = parser.parse_args()
    source_endpoint = Endpoint(arguments.source_container, arguments.source_exec_user)
    target_endpoint = Endpoint(arguments.target_container, arguments.target_exec_user)
    if (arguments.source_container == arguments.target_container
            and arguments.source_database == arguments.into):
        fail("source and target name the same database")

    source = columns(arguments.source_database, arguments.company, source_endpoint)
    if not source:
        fail(f"{SOURCE}.\"{arguments.company}\" has no columns at all")

    target = columns(arguments.into, "public", target_endpoint)
    target.pop('agiru_seed_provenance', None)
    if not target:
        fail(f"{arguments.into}.public has no columns at all. The schema is made by the runner's "
             f"own ProvisionInstalled, so run `agiru run-tests --codeunit <any>` once first.")
    by_fold = {}
    for name in target:
        key = folded(name)
        if key in by_fold:
            fail(f"ambiguous target table fold: {by_fold[key]} and {name}")
        by_fold[key] = name
    shared = sorted((theirs, by_fold[theirs]) for theirs in source if theirs in by_fold)
    if not shared:
        fail("no table exists in both. ABORT, not a pass.")

    details = provenance(arguments.company, source, target, arguments.into,
                         arguments.source_database, source_endpoint, target_endpoint)
    if arguments.verify:
        resume_seed(arguments.into, details, target_endpoint)
    else:
        begin_seed(arguments.into, details, target_endpoint)
    print(f"seed: {len(source)} source table(s), {len(target)} target table(s), "
          f"{len(shared)} shared table(s)", flush=True)

    seeded = 0
    rows = 0
    empty = 0
    refused = []
    dropped = 0
    defaulted = 0
    dropped_columns = []
    defaulted_columns = []
    for index, (theirs, ours) in enumerate(shared):
        if index % 100 == 0:
            print(f"seed: processed {index}/{len(shared)} table(s), {rows} row(s), "
                  f"{len(refused)} refusal(s)", flush=True)
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
        paired_source = {name for name, _ in pairs}
        paired_target = {name for _, name in pairs}
        dropped_columns.extend((theirs, name) for name in base_columns + extra
                               if name not in paired_source)
        defaulted_columns.extend((ours, name) for name in target[ours]
                                 if name not in paired_target)
        if not pairs:
            continue
        table = ours
        projected = [
            blanked(arguments.source_database, companion if p[0] in extra else theirs,
                    p[0], column_kind(arguments.into, ours, p[1], target_endpoint),
                    "e." if p[0] in extra else "b.", source_endpoint)
            for p in pairs]
        selected = ', '.join(projected)
        joined = ""
        if any(p[0] in extra for p in pairs):
            key = [c for c in base_columns
                   if c in source[companion] and c != "timestamp" and not c.startswith("$")]
            if not key:
                refused.append((table, "the companion shares no key column"))
                continue
            joined = (f' LEFT JOIN {quoted([arguments.company])}.{quoted([companion])} e ON ' +
                      " AND ".join(f'b.{quoted([c])} = e.{quoted([c])}' for c in key))
        reading = ('\\copy (SELECT ' + selected +
                   f' FROM {quoted([arguments.company])}.{quoted([theirs])} b{joined}) TO STDOUT')
        writing = '\\copy public.' + quoted([ours]) + ' (' + quoted(p[1] for p in pairs) + ') FROM STDIN'
        try:
            if arguments.verify:
                kinds = [column_kind(arguments.into, ours, p[1], target_endpoint) for p in pairs]
                source_values = ', '.join(canonical(value, kind)
                                          for value, kind in zip(projected, kinds))
                target_values = ', '.join(canonical(quoted([pair[1]]), kind)
                                          for pair, kind in zip(pairs, kinds))
                reading = ('\\copy (SELECT to_jsonb(ROW(' + source_values + '))'
                           f' FROM {quoted([arguments.company])}.{quoted([theirs])} b{joined}) TO STDOUT')
                writing = ('\\copy (SELECT to_jsonb(ROW(' + target_values + '))'
                           f' FROM public.{quoted([ours])}) TO STDOUT')
                source_status, target_status, note = verify_rows(
                    reading, writing, arguments.source_database, arguments.into,
                    source_endpoint, target_endpoint)
            else:
                source_status, target_status, note = transfer(
                    reading, writing, arguments.into, arguments.source_database,
                    source_endpoint, target_endpoint)
        except OSError as error:
            refused.append((table, f"the transfer could not start: {error}"))
            continue
        if source_status != 0 or target_status != 0:
            first = [l for l in note.splitlines() if l.startswith("ERROR:")]
            reason = first[0] if first else note.strip()[:120]
            refused.append((table, f"reader {source_status}, writer {target_status}: {reason}"))
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
    details.update({'tables_seeded': seeded, 'rows_seeded': rows, 'empty_tables': empty,
                    'source_only_tables': len(source) - len(shared),
                    'target_only_tables': len(target) - len(shared),
                    'dropped_columns': dropped, 'defaulted_columns': defaulted,
                    'unmatched_source_tables': sorted(set(source) - {a for a, _ in shared}),
                    'unmatched_target_tables': sorted(set(target) - {b for _, b in shared}),
                    'unmatched_source_columns': dropped_columns,
                    'unmatched_target_columns': defaulted_columns,
                    'refused_tables': refused,
                    'typed_readback': arguments.verify})
    if rows:
        reconcile_rowversions(arguments.into, target, target_endpoint)
    finish_seed(arguments.into, details, target_endpoint,
                status='failed' if refused or rows == 0 else 'complete')
    if rows == 0:
        fail("not one row went in. ABORT, not a pass.")
    if refused:
        fail(f"{len(refused)} table(s) refused; the seed is incomplete")


if __name__ == "__main__":
    main()
