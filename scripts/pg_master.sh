#!/bin/sh
# The PostgreSQL instance and the master database.
#
# WHY A MASTER DATABASE. A test run needs an untouched dataset, and re-seeding it per run costs
# minutes. `CREATE DATABASE ... TEMPLATE agiru_master` copies it at the file level in seconds. The
# master is not written to again after it is filled.
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

podman exec "$CTR" psql -U agiru -tAc \
  "SELECT 1 FROM pg_database WHERE datname='$MASTER'" | grep -q 1 || \
  podman exec "$CTR" createdb -U agiru "$MASTER"

printf 'pg: %s is up, max_locks_per_transaction=%s\n' "$MASTER" "$locks"
printf 'pg: DATABASE_URL=postgresql://agiru:%s@localhost:%s/%s\n' "$PW" "$PORT" "$MASTER"
