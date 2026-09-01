#!/bin/sh
# Die Demo-Datenbank in einen SQL-Server-Container einspielen. Der Container ist eine Durchreiche:
# er haelt die Daten nur, bis sie in PostgreSQL stehen.
set -eu
cd "$(dirname "$0")/.."

CTR=${AGIRU_MSSQL_CTR:-agiru-mssql}
PW=${AGIRU_MSSQL_PASSWORD:-Agiru!Pass2026}
WORK=$(pwd)/work
BAK=$WORK/cronus.bak
[ -f "$BAK" ] || { printf 'mssql: %s fehlt -- erst scripts/fetch_artifact.sh\n' "$BAK" >&2; exit 1; }

if ! podman container exists "$CTR" 2>/dev/null; then
  printf 'mssql: lege %s an\n' "$CTR"
  podman run -d --name "$CTR" \
    -e ACCEPT_EULA=Y -e MSSQL_SA_PASSWORD="$PW" -e MSSQL_MEMORY_LIMIT_MB=4096 \
    -p 1433:1433 -v "$WORK":/bak:z \
    mcr.microsoft.com/mssql/server:2022-latest >/dev/null
else
  podman start "$CTR" >/dev/null
fi

# DAS WERKZEUG WIRD GESUCHT, NICHT ANGENOMMEN. Der Pfad zu sqlcmd hat sich zwischen den Images
# schon einmal verschoben (mssql-tools -> mssql-tools18), und ein fester Pfad faellt dann als
# "Container startet nicht" auf, was er nicht ist.
SQLCMD=$(podman exec "$CTR" sh -c 'ls /opt/mssql-tools*/bin/sqlcmd 2>/dev/null | head -1')
[ -n "$SQLCMD" ] || { printf 'mssql: kein sqlcmd im Image\n' >&2; exit 1; }

printf 'mssql: warte auf Verbindungen'
i=0
until podman exec "$CTR" "$SQLCMD" -S localhost -U sa -P "$PW" -C -Q "SELECT 1" >/dev/null 2>&1; do
  i=$((i + 1)); [ "$i" -gt 90 ] && { printf '\nmssql: kommt nicht hoch\n' >&2; exit 1; }
  printf '.'; sleep 2
done
printf ' nach %ss\n' "$((i * 2))"

# DIE LOGISCHEN DATEINAMEN WERDEN GELESEN, NICHT GERATEN. Sie tragen die Hauptversion im Namen
# ("Demo Database BC (28-0)_Data") und aendern sich mit jedem Release.
files=$(podman exec "$CTR" "$SQLCMD" -S localhost -U sa -P "$PW" -C -h-1 -W -s'|' \
  -Q "SET NOCOUNT ON; RESTORE FILELISTONLY FROM DISK='/bak/cronus.bak'")
data=$(printf '%s\n' "$files" | awk -F'|' '$3=="D"{print $1; exit}')
log=$(printf '%s\n' "$files"  | awk -F'|' '$3=="L"{print $1; exit}')
[ -n "$data" ] && [ -n "$log" ] || { printf 'mssql: FILELISTONLY unlesbar:\n%s\n' "$files" >&2; exit 1; }
printf 'mssql: logische Dateien "%s" / "%s"\n' "$data" "$log"

podman exec "$CTR" "$SQLCMD" -S localhost -U sa -P "$PW" -C -Q "
RESTORE DATABASE CRONUS FROM DISK='/bak/cronus.bak'
WITH MOVE '$data' TO '/var/opt/mssql/data/CRONUS.mdf',
     MOVE '$log'  TO '/var/opt/mssql/data/CRONUS.ldf', REPLACE, RECOVERY;"

podman exec "$CTR" "$SQLCMD" -S localhost -U sa -P "$PW" -C -d CRONUS -h-1 -W \
  -Q "SET NOCOUNT ON; SELECT CONCAT(COUNT(*), ' Tabellen in CRONUS') FROM sys.tables;"
