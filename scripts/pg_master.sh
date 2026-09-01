#!/bin/sh
# Die PostgreSQL-Instanz und die Master-Datenbank.
#
# WARUM EINE MASTER-DATENBANK. Ein Testlauf braucht einen unberuehrten Bestand, und ihn je Lauf neu
# zu seeden kostet Minuten. `CREATE DATABASE ... TEMPLATE agiru_master` kopiert ihn auf Dateiebene
# in Sekunden. Der Master wird nach dem Befuellen nicht mehr beschrieben.
#
# max_locks_per_transaction: das BC-Schema hat rund 1 600 Tabellen und ebenso viele Indizes. Eine
# All-in-one-Transaktion nimmt ein Lock je Objekt und sprengt den Default von 64 mit
# "out of shared memory". Der Wert steht als Kommandozeilenargument und nicht in
# postgresql.auto.conf, weil der die Neuanlage des Containers nicht ueberlebt.
set -eu

CTR=${AGIRU_PG_CTR:-agiru-pg}
PORT=${AGIRU_PG_PORT:-5433}
PW=${AGIRU_PG_PASSWORD:-agiru}
MASTER=${AGIRU_PG_MASTER:-agiru_master}

if ! podman container exists "$CTR" 2>/dev/null; then
  printf 'pg: lege %s an\n' "$CTR"
  podman run -d --name "$CTR" \
    -e POSTGRES_USER=agiru -e POSTGRES_PASSWORD="$PW" -e POSTGRES_DB=agiru \
    -p "$PORT":5432 docker.io/library/postgres:17 \
    -c max_locks_per_transaction=1024 -c shared_buffers=512MB >/dev/null
else
  podman start "$CTR" >/dev/null
fi

printf 'pg: warte auf Verbindungen'
i=0
until podman exec "$CTR" pg_isready -U agiru >/dev/null 2>&1; do
  i=$((i + 1)); [ "$i" -gt 60 ] && { printf '\npg: kommt nicht hoch\n' >&2; exit 1; }
  printf '.'; sleep 1
done
printf ' nach %ss\n' "$i"

locks=$(podman exec "$CTR" psql -U agiru -tAc 'SHOW max_locks_per_transaction')
[ "$locks" -ge 1024 ] || { printf 'pg: max_locks_per_transaction=%s, gebraucht werden 1024\n' "$locks" >&2; exit 1; }

podman exec "$CTR" psql -U agiru -tAc \
  "SELECT 1 FROM pg_database WHERE datname='$MASTER'" | grep -q 1 || \
  podman exec "$CTR" createdb -U agiru "$MASTER"

printf 'pg: %s steht, max_locks_per_transaction=%s\n' "$MASTER" "$locks"
printf 'pg: DATABASE_URL=postgresql://agiru:%s@localhost:%s/%s\n' "$PW" "$PORT" "$MASTER"
