#!/bin/sh
# `sh scripts/pg_seal.sh open|close` -- lift and restore the master's seal.
#
# THE MASTER IS A TEMPLATE AND A TEMPLATE IS NOT CONNECTABLE. `datallowconn = false` is how
# PostgreSQL keeps `template0` pristine, and it is the only guard that cannot be forgotten: with it
# set, no run, no gate and no stray psql can write to the master, and none of them can be the
# session that makes a clone fail with "source database is being accessed by other users".
#
# It is lifted to FILL the master -- the schema the transpiler emits, the CRONUS load -- and put
# back afterwards. A master left open is not a broken build; it is a master that quietly drifts
# from what every clone since was made of.
set -eu

CTR=${AGIRU_PG_CTR:-agiru-pg}
MASTER=${AGIRU_PG_MASTER:-agiru_master}

case "${1:-}" in
  open)  allow=true;  what=open ;;
  close) allow=false; what=sealed ;;
  *) printf 'usage: %s open|close\n' "$0" >&2; exit 2 ;;
esac

podman exec "$CTR" psql -U agiru -d postgres -qc \
  "ALTER DATABASE \"$MASTER\" IS_TEMPLATE true ALLOW_CONNECTIONS $allow" >/dev/null
printf 'pg: %s is %s\n' "$MASTER" "$what"
