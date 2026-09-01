#!/bin/sh
# `make provision` -- die Quelle, das Artefakt, der SQL Server, die PostgreSQL.
#
# DIE DEMO-DATENBANK MUSS ZUR QUELLE PASSEN. Ein Schema aus der einen Version und Daten aus der
# anderen erzeugen ein Fehlerbild, das wie ein Runtime-Defekt aussieht und keiner ist -- fehlende
# Spalten, unbekannte Felder, Enums mit verschobenen Ordinalen. Deshalb steht die Version an EINER
# Stelle (BC_VERSION) und dieses Skript verweigert, wenn die AL-Quelle woanders steht.
set -eu
cd "$(dirname "$0")/.."

VERSION=$(cat BC_VERSION)
WANT=$(printf '%s' "$VERSION" | cut -d. -f1,2)
BCAPPS=${BCAPPS:-$HOME/Git/BCApps}
TREE=work/bcapps-$WANT

[ -d "$BCAPPS/.git" ] || { printf 'provision: %s ist kein Git-Baum\n' "$BCAPPS" >&2; exit 1; }

# EIN WORKTREE STATT EINES CHECKOUTS. Der Arbeitsstand des Nutzers in ~/Git/BCApps bleibt, wo er
# ist; agiru bekommt seinen eigenen, festgenagelten Baum. Rueckgaengig mit `git worktree remove`.
if [ ! -d "$TREE" ]; then
  printf 'provision: lege Worktree %s auf releases/%s an\n' "$TREE" "$WANT"
  mkdir -p work
  git -C "$BCAPPS" fetch --quiet origin "releases/$WANT" || true
  git -C "$BCAPPS" worktree add --quiet --detach "$(pwd)/$TREE" "origin/releases/$WANT"
fi

have=$(python3 -c "import json,sys;print(json.load(open(sys.argv[1]))['version'])" \
       "$TREE/src/System Application/App/app.json")
case "$have" in
  "$WANT".*) : ;;
  *) printf 'provision: %s traegt %s, BC_VERSION verlangt %s.x\n' "$TREE" "$have" "$WANT" >&2
     rm -rf "$TREE"; printf 'provision: Worktree entfernt. BC_VERSION pruefen.\n' >&2; exit 1 ;;
esac
printf 'provision: AL-Quelle %s auf %s\n' "$TREE" "$have"

sh scripts/fetch_artifact.sh
sh scripts/mssql_restore.sh
sh scripts/pg_master.sh

printf '\nprovision: fertig. Die Uebertragung MSSQL -> PostgreSQL braucht das Schema, das der\n'
printf 'provision: Transpiler erst erzeugt -- board:0004.\n'
