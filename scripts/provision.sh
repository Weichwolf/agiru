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
WANT=$(printf '%s' "$VERSION" | cut -d. -f1,2)
BCAPPS=${BCAPPS:-$HOME/Git/BCApps}
TREE=work/bcapps-$WANT

[ -d "$BCAPPS/.git" ] || { printf 'provision: %s is not a git tree\n' "$BCAPPS" >&2; exit 1; }

# A WORKTREE RATHER THAN A CHECKOUT. The user's working state in ~/Git/BCApps stays where it is;
# agiru gets its own pinned tree. Undo with `git worktree remove`.
if [ ! -d "$TREE" ]; then
  printf 'provision: creating worktree %s on releases/%s\n' "$TREE" "$WANT"
  mkdir -p work
  git -C "$BCAPPS" fetch --quiet origin "releases/$WANT" || true
  git -C "$BCAPPS" worktree add --quiet --detach "$(pwd)/$TREE" "origin/releases/$WANT"
fi

have=$(python3 -c "import json,sys;print(json.load(open(sys.argv[1]))['version'])" \
       "$TREE/src/System Application/App/app.json")
case "$have" in
  "$WANT".*) : ;;
  *) printf 'provision: %s carries %s, BC_VERSION asks for %s.x\n' "$TREE" "$have" "$WANT" >&2
     rm -rf "$TREE"; printf 'provision: worktree removed. Check BC_VERSION.\n' >&2; exit 1 ;;
esac
printf 'provision: AL source %s at %s\n' "$TREE" "$have"

sh scripts/fetch_artifact.sh
sh scripts/mssql_restore.sh
sh scripts/pg_master.sh

printf '\nprovision: done. The MSSQL -> PostgreSQL transfer needs the schema the transpiler has\n'
printf 'provision: yet to emit -- board:0004.\n'
