#!/usr/bin/env python3
"""Fetch the AL declaration of every PLATFORM object from the system symbols.

WHY THIS EXISTS. The platform objects -- `Company`, `User`, `Field`, `Date`, `Integer`, `Session`,
`AllObj` and 270 others -- are declared by no file in `~/Git/BCApps`: BC ships them with the
platform, and until now this tree had to guess their field numbers from a column order or from the
predecessor. Both guesses were wrong. `System.app`, the symbol package the AL compiler compiles
BCApps against, carries the AL SOURCE of all of them, and it is inside the on-prem PLATFORM
artefact on the same CDN the demo database comes from.

THE ARTEFACT IS 1.37 GB AND ONE ENTRY OF IT IS 0.6 MB, so this reads it over HTTP range requests:
the end-of-central-directory record, then the central directory, then the one local entry. Nothing
else is downloaded.

`System.app` IS A NAVX CONTAINER: a 40-byte header, then an ordinary zip whose `src/` holds one
`.al` per platform object. The directory names inside are encoded TWICE (`%2520` is `%20` encoded
again), so a segment is unquoted twice on the way out.
"""

import base64
import hashlib
import pathlib
import struct
import subprocess
import sys
import urllib.parse
import zlib

CDN = "https://bcartifacts-exdbf9fwegejdqak.b02.azurefd.net"
ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "work" / "symbols"
TAIL = 3_000_000
NAVX = b"NAVX"


def fail(message):
    print(f"symbols: {message}", file=sys.stderr)
    raise SystemExit(1)


def curl(url, first=None, last=None):
    argv = ["curl", "-fsS"]
    if first is not None:
        argv += ["-H", f"Range: bytes={first}-{last}"]
    done = subprocess.run(argv + [url], capture_output=True, check=False)
    if done.returncode != 0:
        fail(f"{url} did not answer: {done.stderr.decode('utf-8', 'replace').strip()}")
    if first is not None and len(done.stdout) != last - first + 1:
        fail(f"the CDN ignored the range request -- asked for {last - first + 1} bytes, "
             f"got {len(done.stdout)}. Without ranges this is a 1.37 GB download and it refuses "
             f"rather than making one.")
    return done.stdout


def size_of(url):
    head = subprocess.run(["curl", "-fsSI", url], capture_output=True, check=False)
    if head.returncode != 0:
        fail(f"{url} has no headers: {head.stderr.decode('utf-8', 'replace').strip()}")
    for line in head.stdout.decode("utf-8", "replace").splitlines():
        name, _, value = line.partition(":")
        if name.strip().lower() == "content-length":
            return int(value.strip())
    fail("the CDN names no Content-Length, so the central directory cannot be located")
    return 0


def central_directory(url, total):
    tail = curl(url, max(0, total - TAIL), total - 1)
    at = tail.rfind(b"PK\x05\x06")
    if at < 0:
        fail("no end-of-central-directory record in the last 3 MB")
    entries, size, offset = struct.unpack_from("<HII", tail, at + 10)
    if 0xFFFFFFFF in (size, offset):
        fail("the artefact is a zip64 archive, which this reader does not parse")
    return entries, curl(url, offset, offset + size - 1)


def entry_named(directory, entries, suffix):
    at = 0
    found = []
    for _ in range(entries):
        if directory[at:at + 4] != b"PK\x01\x02":
            fail(f"the central directory breaks at byte {at}")
        compressed, = struct.unpack_from("<I", directory, at + 20)
        nlen, elen, clen = struct.unpack_from("<HHH", directory, at + 28)
        local, = struct.unpack_from("<I", directory, at + 42)
        name = directory[at + 46:at + 46 + nlen].decode("utf-8", "replace")
        if name.lower().endswith(suffix):
            found.append((name, local, compressed))
        at += 46 + nlen + elen + clen
    if len(found) != 1:
        fail(f"{len(found)} entries end in {suffix!r}, and exactly one was expected")
    return found[0]


def member(url, local, compressed):
    head = curl(url, local, local + 29)
    if head[:4] != b"PK\x03\x04":
        fail(f"byte {local} is not a local file header")
    method, = struct.unpack_from("<H", head, 8)
    nlen, elen = struct.unpack_from("<HH", head, 26)
    first = local + 30 + nlen + elen
    blob = curl(url, first, first + compressed - 1)
    if method == 0:
        return blob
    if method != 8:
        fail(f"compression method {method} is neither stored nor deflate")
    return zlib.decompress(blob, -15)


def unpack(app, into):
    if app[:4] != NAVX:
        fail(f"the package does not begin with {NAVX.decode()} -- it is not a symbol package")
    at = app.find(b"PK\x03\x04")
    if at < 0:
        fail("the NAVX container holds no zip")
    archive = into.parent / "System.zip"
    into.parent.mkdir(parents=True, exist_ok=True)
    archive.write_bytes(app[at:])
    if into.exists():
        subprocess.run(["rm", "-rf", str(into)], check=True)
    into.mkdir(parents=True)
    done = subprocess.run(["unzip", "-qq", str(archive), "-d", str(into)], check=False)
    if done.returncode != 0:
        fail(f"unzip refused {archive}")
    written = 0
    for path in sorted(into.rglob("*"), key=lambda p: -len(p.parts)):
        plain = urllib.parse.unquote(urllib.parse.unquote(path.name))
        if plain != path.name:
            path.rename(path.parent / plain)
        written += path.is_file()
    return written


def main():
    version = (ROOT / "BC_VERSION").read_text().strip()
    url = f"{CDN}/onprem/{version}/platform"
    total = size_of(url)
    print(f"symbols: {url} is {total / 1e9:.2f} GB, and this reads three ranges of it")
    entries, directory = central_directory(url, total)
    name, local, compressed = entry_named(directory, entries, "system.app")
    print(f"symbols: {name} at {local}, {compressed} bytes compressed")
    app = member(url, local, compressed)
    digest = base64.b64encode(hashlib.sha256(app).digest()).decode()
    print(f"symbols: {len(app)} bytes, SHA-256 {digest}")
    count = unpack(app, OUT)
    declarations = len(list(OUT.rglob("*.al")))
    if declarations == 0:
        fail("the package carries no .al file, so it is not the source-bearing symbols")
    print(f"symbols: {count} files under {OUT}, {declarations} of them AL declarations")


if __name__ == "__main__":
    main()
