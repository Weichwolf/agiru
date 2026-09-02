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

# Measured 2026-09-02: Front Door sends no Content-MD5. The CRC path is the one that runs.
want=$(curl -fsSI "$URL" | awk 'tolower($1)=="content-md5:"{print $2}' | tr -d '\r')
have=$(openssl dgst -md5 -binary "$ZIP" | base64) || {
  printf 'artefact: cannot compute an MD5 of %s -- refusing to call it verified\n' "$ZIP" >&2
  exit 1
}
if [ -z "$want" ]; then
  printf 'artefact: the CDN names no checksum; verifying the archive against its own CRCs\n'
  unzip -tqq "$ZIP" || {
    printf 'artefact: %s does not check out against its own CRCs -- delete it and fetch again\n' \
      "$ZIP" >&2
    exit 1
  }
  printf 'artefact: %s, CRCs check out\n' "$(du -h "$ZIP" | cut -f1)"
elif [ "$want" != "$have" ]; then
  printf 'artefact: MD5 differs -- the CDN says %s, the file has %s\n' "$want" "$have" >&2
  printf 'artefact: delete %s and download again.\n' "$ZIP" >&2
  exit 1
else
  printf 'artefact: %s, MD5 %s checks out\n' "$(du -h "$ZIP" | cut -f1)" "$want"
fi

BAK="$WORK/cronus.bak"
if [ ! -f "$BAK" ]; then
  inner=$(unzip -Z1 "$ZIP" | grep -i '\.bak$' | head -1)
  [ -n "$inner" ] || { printf 'artefact: no .bak inside the zip\n' >&2; exit 1; }
  printf 'artefact: extracting "%s"\n' "$inner"
  # THE ENTRY NAME CARRIES A BACKSLASH -- the zip was written on Windows and the separator is stored
  # as `database\Demo Database BC (28-0).bak`. `unzip` reads a member name as a PATTERN in which
  # \ * ? [ ] are special, so the raw name matches nothing, `unzip -p` exits 11 and writes an empty
  # file. Escaping the four characters is what makes the name mean itself.
  pattern=$(printf '%s' "$inner" | sed 's/[][*?\\]/\\&/g')
  # Put it under an ASCII name without spaces or brackets: the path goes on as a container mount
  # and as an SQL literal, and neither likes the original name.
  unzip -p "$ZIP" "$pattern" > "$BAK.part"
  # AN EMPTY EXTRACTION IS NOT AN EXTRACTION. Renaming it would hand the restore a zero-byte backup
  # and the failure would surface two steps later as an SQL error about a corrupt media set.
  [ -s "$BAK.part" ] || {
    printf 'artefact: extracting "%s" produced nothing\n' "$inner" >&2
    rm -f "$BAK.part"
    exit 1
  }
  mv "$BAK.part" "$BAK"
fi
printf 'artefact: %s lies at %s\n' "$(du -h "$BAK" | cut -f1)" "$BAK"
