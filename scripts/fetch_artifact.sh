#!/bin/sh
# Das BC-OnPrem-Artefakt vom Microsoft-CDN holen und die Demo-Datenbank herausloesen.
#
# WELCHER HOST. `bcartifacts.blob.core.windows.net` beantwortet nichts mehr ("not authorized by
# network security perimeter", gemessen), `bcartifacts.azureedge.net` loest nicht mehr auf. Was
# antwortet, ist Front Door.
#
# DAS ARTEFAKT IST EIN ZIP MIT DREI EINTRAEGEN (gemessen ueber einen Range-Request auf das zentrale
# Verzeichnis, ohne die 372 MB zu laden):
#     824 299 520  database/Demo Database BC (28-0).bak
#          31 000  database/Cronus.bclicense
#             205  manifest.json
# Der Name im Zip traegt "(28-0)" auch fuer 28.4 -- die Nebenversion steht nicht darin.
set -eu
cd "$(dirname "$0")/.."

CDN=https://bcartifacts-exdbf9fwegejdqak.b02.azurefd.net
VERSION=$(cat BC_VERSION)
COUNTRY=${BC_COUNTRY:-w1}
WORK=work
ZIP="$WORK/bc-$VERSION-$COUNTRY.zip"

mkdir -p "$WORK"
URL="$CDN/onprem/$VERSION/$COUNTRY"

if [ ! -f "$ZIP" ]; then
  printf 'artefakt: lade %s\n' "$URL"
  curl -fL --progress-bar -o "$ZIP.part" "$URL"
  mv "$ZIP.part" "$ZIP"
fi

# DER CDN NENNT SEINE EIGENE PRUEFSUMME. Sie zu holen kostet einen HEAD; sie NICHT zu pruefen
# kostet einen halben Download, der als Restore-Fehler zurueckkommt und wie ein Datenfehler aussieht.
want=$(curl -fsSI "$URL" | awk 'tolower($1)=="content-md5:"{print $2}' | tr -d '\r')
have=$(md5sum "$ZIP" | cut -d' ' -f1 | xxd -r -p | base64)
if [ -n "$want" ] && [ "$want" != "$have" ]; then
  printf 'artefakt: MD5 weicht ab -- CDN sagt %s, die Datei hat %s\n' "$want" "$have" >&2
  printf 'artefakt: %s loeschen und neu laden.\n' "$ZIP" >&2
  exit 1
fi
printf 'artefakt: %s, MD5 %s stimmt\n' "$(du -h "$ZIP" | cut -f1)" "${want:-ungeprueft}"

BAK="$WORK/cronus.bak"
if [ ! -f "$BAK" ]; then
  inner=$(unzip -Z1 "$ZIP" | grep -i '\.bak$' | head -1)
  [ -n "$inner" ] || { printf 'artefakt: kein .bak im Zip\n' >&2; exit 1; }
  printf 'artefakt: entpacke "%s"\n' "$inner"
  # Unter einen ASCII-Namen ohne Leerzeichen und Klammern legen: der Pfad geht gleich als
  # Container-Mount und als SQL-Literal weiter, und beides mag den Originalnamen nicht.
  unzip -p "$ZIP" "$inner" > "$BAK.part"
  mv "$BAK.part" "$BAK"
fi
printf 'artefakt: %s liegt als %s\n' "$(du -h "$BAK" | cut -f1)" "$BAK"
