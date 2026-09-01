#!/bin/sh
# Fetch the BC on-prem artefact from the Microsoft CDN and extract the demo database.
#
# WHICH HOST. `bcartifacts.blob.core.windows.net` answers nothing any more ("not authorized by
# network security perimeter", measured); `bcartifacts.azureedge.net` no longer resolves. What
# answers is Front Door.
#
# THE ARTEFACT IS A ZIP WITH THREE ENTRIES (measured over a range request on the central directory,
# without downloading the 372 MB):
#     824 299 520  database/Demo Database BC (28-0).bak
#          31 000  database/Cronus.bclicense
#             205  manifest.json
# The name inside the zip carries "(28-0)" even for 28.4 -- the minor version is not in it.
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
  printf 'artefact: downloading %s\n' "$URL"
  curl -fL --progress-bar -o "$ZIP.part" "$URL"
  mv "$ZIP.part" "$ZIP"
fi

# THE CDN NAMES ITS OWN CHECKSUM. Fetching it costs one HEAD; NOT checking it costs half a download
# that comes back as a restore error and looks like a data defect.
want=$(curl -fsSI "$URL" | awk 'tolower($1)=="content-md5:"{print $2}' | tr -d '\r')
have=$(md5sum "$ZIP" | cut -d' ' -f1 | xxd -r -p | base64)
if [ -n "$want" ] && [ "$want" != "$have" ]; then
  printf 'artefact: MD5 differs -- the CDN says %s, the file has %s\n' "$want" "$have" >&2
  printf 'artefact: delete %s and download again.\n' "$ZIP" >&2
  exit 1
fi
printf 'artefact: %s, MD5 %s checks out\n' "$(du -h "$ZIP" | cut -f1)" "${want:-unverified}"

BAK="$WORK/cronus.bak"
if [ ! -f "$BAK" ]; then
  inner=$(unzip -Z1 "$ZIP" | grep -i '\.bak$' | head -1)
  [ -n "$inner" ] || { printf 'artefact: no .bak inside the zip\n' >&2; exit 1; }
  printf 'artefact: extracting "%s"\n' "$inner"
  # Put it under an ASCII name without spaces or brackets: the path goes on as a container mount
  # and as an SQL literal, and neither likes the original name.
  unzip -p "$ZIP" "$inner" > "$BAK.part"
  mv "$BAK.part" "$BAK"
fi
printf 'artefact: %s lies at %s\n' "$(du -h "$BAK" | cut -f1)" "$BAK"
