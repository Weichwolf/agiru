#!/usr/bin/env bash
set -euo pipefail
postgres_pid=
application_pid=
proxy_pid=
finish() {
  trap - TERM INT EXIT
  if [[ -n "$application_pid" ]] && kill -0 "$application_pid" 2>/dev/null; then
    kill -TERM "$application_pid"
    wait "$application_pid" || :
  fi
  if [[ -n "$proxy_pid" ]] && kill -0 "$proxy_pid" 2>/dev/null; then
    kill -TERM "$proxy_pid"
    wait "$proxy_pid" || :
  fi
  if [[ -n "$postgres_pid" ]]; then
    runuser -u postgres -- pg_ctl -D "$PGDATA" -m fast -w -t 20 stop || :
    wait "$postgres_pid" || :
  fi
}
trap finish EXIT
trap 'exit 143' TERM
trap 'exit 130' INT
await_service_exit() {
  while :; do
    for candidate in "$@"; do
      if ! kill -0 "$candidate" 2>/dev/null; then
        finished_pid=$candidate
        wait "$candidate" || status=$?
        return
      fi
    done
    sleep 0.25
  done
}
if [[ -f "$PGDATA/PG_VERSION" ]]; then
  [[ "$(<"$PGDATA/PG_VERSION")" = 17 ]] || { printf 'PostgreSQL storage version mismatch\n' >&2; exit 1; }
else
  [[ ! -e "$PGDATA" ]] || [[ -d "$PGDATA" && -z "$(ls -A "$PGDATA")" ]] || {
    printf 'Refusing nonempty PostgreSQL storage without PG_VERSION\n' >&2; exit 1;
  }
  install -d -m 700 -o postgres -g postgres "$PGDATA"
  runuser -u postgres -- initdb -D "$PGDATA" --encoding=UTF8 --locale=C.UTF-8 \
    --auth-local=peer --auth-host=scram-sha-256
fi
runuser -u postgres -- postgres -D "$PGDATA" -c listen_addresses=127.0.0.1 &
postgres_pid=$!
for ((attempt=0; attempt<60; attempt++)); do
  if pg_isready -h 127.0.0.1 -p 5432 -U postgres -d postgres >/dev/null; then break; fi
  kill -0 "$postgres_pid" 2>/dev/null || { wait "$postgres_pid"; exit 1; }
  sleep 0.5
done
pg_isready -h 127.0.0.1 -p 5432 -U postgres -d postgres >/dev/null
runuser -u postgres -- psql -X -v ON_ERROR_STOP=1 postgres <<'SQL'
SELECT 'CREATE ROLE agiru LOGIN SUPERUSER PASSWORD ''agiru'''
WHERE NOT EXISTS (SELECT FROM pg_roles WHERE rolname = 'agiru') \gexec
SELECT format('CREATE DATABASE %I OWNER agiru', name)
FROM (VALUES ('agiru_gate'), ('agiru_master')) AS desired(name)
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = desired.name) \gexec
SQL
printf 'Development PostgreSQL ready; credentials are development-only.\n'
install -d -m 755 -o agiru -g agiru /run/agiru /usr/share/agiru/web
install -d -m 700 -o agiru -g agiru "$XDG_DATA_HOME" "$XDG_CONFIG_HOME"
runuser -u agiru -- caddy validate --config /etc/agiru/Caddyfile --adapter caddyfile
setpriv --reuid agiru --regid agiru --init-groups caddy run --config /etc/agiru/Caddyfile \
  --adapter caddyfile --pidfile /run/agiru/caddy.pid &
proxy_pid=$!
status=0
finished_pid=
if [[ "${1:-}" = database-only && "$#" = 1 ]]; then
  await_service_exit "$postgres_pid" "$proxy_pid"
else
  [[ "$#" -gt 0 ]] || { printf 'An application command is required\n' >&2; exit 1; }
  runuser -u agiru -- "$@" &
  application_pid=$!
  await_service_exit "$postgres_pid" "$application_pid" "$proxy_pid"
fi
if [[ "$finished_pid" != "$application_pid" && "$status" = 0 ]]; then status=1; fi
exit "$status"
