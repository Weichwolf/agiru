#!/bin/sh
# Restore the demo database into a SQL Server container. The container is a pass-through: it holds
# the data only until it is in PostgreSQL.
set -eu
cd "$(dirname "$0")/.."

CTR=${AGIRU_MSSQL_CTR:-agiru-mssql}
PW=${AGIRU_MSSQL_PASSWORD:-Agiru!Pass2026}
WORK=$(pwd)/work
BAK=$WORK/cronus.bak
[ -f "$BAK" ] || { printf 'mssql: %s is missing -- run scripts/fetch_artifact.sh first\n' "$BAK" >&2; exit 1; }

if ! podman container exists "$CTR" 2>/dev/null; then
  printf 'mssql: creating %s\n' "$CTR"
  podman run -d --name "$CTR" \
    -e ACCEPT_EULA=Y -e MSSQL_SA_PASSWORD="$PW" -e MSSQL_MEMORY_LIMIT_MB=4096 \
    -p 1433:1433 -v "$WORK":/bak:z \
    mcr.microsoft.com/mssql/server:2022-latest >/dev/null
else
  podman start "$CTR" >/dev/null
fi

# THE TOOL IS LOOKED FOR, NOT ASSUMED. The path to sqlcmd has moved between images once already
# (mssql-tools -> mssql-tools18), and a fixed path then shows up as "the container will not start",
# which it is not.
SQLCMD=$(podman exec "$CTR" sh -c 'ls /opt/mssql-tools*/bin/sqlcmd 2>/dev/null | head -1')
[ -n "$SQLCMD" ] || { printf 'mssql: no sqlcmd in the image\n' >&2; exit 1; }

printf 'mssql: waiting for connections'
i=0
until podman exec "$CTR" "$SQLCMD" -S localhost -U sa -P "$PW" -C -Q "SELECT 1" >/dev/null 2>&1; do
  i=$((i + 1)); [ "$i" -gt 90 ] && { printf '\nmssql: does not come up\n' >&2; exit 1; }
  printf '.'; sleep 2
done
printf ' after %ss\n' "$((i * 2))"

# THE LOGICAL FILE NAMES ARE READ, NOT GUESSED. They carry the major version in the name
# ("Demo Database BC (28-0)_Data") and change with every release.
files=$(podman exec "$CTR" "$SQLCMD" -S localhost -U sa -P "$PW" -C -h-1 -W -s'|' \
  -Q "SET NOCOUNT ON; RESTORE FILELISTONLY FROM DISK='/bak/cronus.bak'")
data=$(printf '%s\n' "$files" | awk -F'|' '$3=="D"{print $1; exit}')
log=$(printf '%s\n' "$files"  | awk -F'|' '$3=="L"{print $1; exit}')
[ -n "$data" ] && [ -n "$log" ] || { printf 'mssql: FILELISTONLY unreadable:\n%s\n' "$files" >&2; exit 1; }
printf 'mssql: logical files "%s" / "%s"\n' "$data" "$log"

podman exec "$CTR" "$SQLCMD" -S localhost -U sa -P "$PW" -C -Q "
RESTORE DATABASE CRONUS FROM DISK='/bak/cronus.bak'
WITH MOVE '$data' TO '/var/opt/mssql/data/CRONUS.mdf',
     MOVE '$log'  TO '/var/opt/mssql/data/CRONUS.ldf', REPLACE, RECOVERY;"

podman exec "$CTR" "$SQLCMD" -S localhost -U sa -P "$PW" -C -d CRONUS -h-1 -W \
  -Q "SET NOCOUNT ON; SELECT CONCAT(COUNT(*), ' tables in CRONUS') FROM sys.tables;"
