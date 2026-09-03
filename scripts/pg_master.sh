#!/bin/sh
# The PostgreSQL instance and the master database.
#
# WHY A MASTER DATABASE. A test run needs an untouched dataset, and re-seeding it per run costs
# minutes. `CREATE DATABASE ... TEMPLATE agiru_master` copies it at the file level in 530-643 ms
# against 1 412 ms for building the schema statement by statement (measured 2026-09-03, 1 609
# tables, 73 MB, PostgreSQL 17.11). The master is not written to again after it is filled.
#
# WHY A SECOND DATABASE FOR THE GATE. PostgreSQL refuses to copy a database ANY session holds open,
# so a gate connected to the master is a gate that stops every clone -- and what a gate leaves in
# the master, every later clone inherits. The predecessor ran both against one database and spent a
# session on eleven failures that belonged to the seed and to neither the code nor the fix under
# measurement (openerp WI-832).
#
# max_locks_per_transaction: the BC schema has some 1 600 tables and as many indexes. An all-in-one
# transaction takes one lock per object and blows the default of 64 with "out of shared memory".
# The value is a command-line argument rather than postgresql.auto.conf, because that does not
# survive recreating the container.
set -eu

CTR=${AGIRU_PG_CTR:-agiru-pg}
PORT=${AGIRU_PG_PORT:-5433}
PW=${AGIRU_PG_PASSWORD:-agiru}
MASTER=${AGIRU_PG_MASTER:-agiru_master}
GATE=${AGIRU_PG_GATE:-agiru_gate}

if ! podman container exists "$CTR" 2>/dev/null; then
  printf 'pg: creating %s\n' "$CTR"
  podman run -d --name "$CTR" \
    -e POSTGRES_USER=agiru -e POSTGRES_PASSWORD="$PW" -e POSTGRES_DB=agiru \
    -p "$PORT":5432 docker.io/library/postgres:17 \
    -c max_locks_per_transaction=1024 -c shared_buffers=512MB >/dev/null
else
  podman start "$CTR" >/dev/null
fi

printf 'pg: waiting for connections'
i=0
until podman exec "$CTR" pg_isready -U agiru >/dev/null 2>&1; do
  i=$((i + 1)); [ "$i" -gt 60 ] && { printf '\npg: does not come up\n' >&2; exit 1; }
  printf '.'; sleep 1
done
printf ' after %ss\n' "$i"

locks=$(podman exec "$CTR" psql -U agiru -tAc 'SHOW max_locks_per_transaction')
[ "$locks" -ge 1024 ] || { printf 'pg: max_locks_per_transaction=%s, 1024 is required\n' "$locks" >&2; exit 1; }

for db in "$MASTER" "$GATE"; do
  podman exec "$CTR" psql -U agiru -tAc \
    "SELECT 1 FROM pg_database WHERE datname='$db'" | grep -q 1 || \
    podman exec "$CTR" createdb -U agiru "$db"
done

# THE MASTER IS SEALED, AND THAT IS NOT A CONVENTION BUT A COLUMN. `datallowconn = false` is what
# PostgreSQL's own `template0` carries: no session can open the database at all, so nothing can
# write to it and no session can be the one that makes `CREATE DATABASE ... TEMPLATE` fail with
# "source database is being accessed by other users". `datistemplate = true` lets a non-superuser
# copy it. Filling the master means lifting the seal, which is `scripts/pg_seal.sh open`.
podman exec "$CTR" psql -U agiru -d postgres -qc \
  "ALTER DATABASE \"$MASTER\" IS_TEMPLATE true ALLOW_CONNECTIONS false" >/dev/null

printf 'pg: %s and %s are up, max_locks_per_transaction=%s\n' "$MASTER" "$GATE" "$locks"
printf 'pg: DATABASE_URL=postgresql://agiru:%s@localhost:%s/%s\n' "$PW" "$PORT" "$MASTER"
