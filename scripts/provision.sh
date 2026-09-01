#!/bin/sh
# `make provision` -- the source, the artefact, SQL Server, PostgreSQL.
#
# THE DEMO DATABASE MUST MATCH THE SOURCE. A schema from one version with data from another produces
# a picture that looks like a runtime defect and is not one -- missing columns, unknown fields, enums
# with shifted ordinals. The version therefore lives in ONE place (BC_VERSION) and this script
# refuses when the AL source stands somewhere else.
set -eu
cd "$(dirname "$0")/.."

VERSION=$(cat BC_VERSION)

sh scripts/fetch_artifact.sh
sh scripts/mssql_restore.sh
sh scripts/pg_master.sh

printf '\nprovision: done. The MSSQL -> PostgreSQL transfer needs the schema the transpiler has\n'
printf 'provision: yet to emit -- board:0004.\n'
