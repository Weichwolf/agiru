#!/usr/bin/env python3
"""Fetch source-bearing System.app by HTTP ranges; preserve NAVX bytes and provenance.

Publish immutable build/symbols/<version>/<SHA256> inputs. Decode each ZIP path
segment twice, refusing traversal, links and collisions before extraction.
"""

import argparse
import hashlib
import io
import json
import pathlib
import re
import stat
import struct
import subprocess
import sys
import tempfile
import urllib.parse
import xml.etree.ElementTree as ET
import zipfile
import zlib

CDN = "https://bcartifacts-exdbf9fwegejdqak.b02.azurefd.net"
ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "build" / "symbols"
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
    entries = {}
    with zipfile.ZipFile(io.BytesIO(app[at:])) as archive:
        for entry in archive.infolist():
            parts = [urllib.parse.unquote(urllib.parse.unquote(part))
                     for part in entry.filename.rstrip('/').split('/')]
            if (any(part in ('', '.', '..') or '/' in part or '\\' in part or '\0' in part
                    for part in parts)
                    or stat.S_ISLNK(entry.external_attr >> 16)):
                fail(f"unsafe package path: {entry.filename!r}")
            relative = pathlib.Path(*parts)
            if relative in entries or relative.parts[0] in ('System.app', 'provenance.json'):
                fail(f"duplicate or reserved package path: {relative}")
            entries[relative] = entry
        for relative in entries:
            for parent in relative.parents:
                if parent in entries and not entries[parent].is_dir():
                    fail(f"package file is also a directory: {parent}")
        into.mkdir(parents=True, exist_ok=False)
        for relative, entry in entries.items():
            path = into / relative
            if entry.is_dir():
                path.mkdir(parents=True, exist_ok=True)
            else:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(archive.read(entry))
    return sum(not entry.is_dir() for entry in entries.values())


def package_identity(into):
    manifest = ET.parse(into / 'NavxManifest.xml')
    app = manifest.getroot().find('{http://schemas.microsoft.com/navx/2015/manifest}App')
    keys = ('Id', 'Name', 'Publisher', 'Version', 'Runtime', 'Target')
    if app is None or any(not app.get(key) for key in keys):
        fail('the package manifest lacks its complete App identity')
    if app.get('Name') != 'System' or app.get('Publisher') != 'Microsoft':
        fail('the package is not Microsoft System')
    with (into / 'SymbolReference.json').open(encoding='utf-8-sig') as source:
        json.load(source)
    if not any((into / 'src').rglob('*.al')):
        fail('the package carries no .al source')
    return {key: app.get(key) for key in keys}


def file_hashes(into):
    return {str(path.relative_to(into)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in sorted(into.rglob('*'))
            if path.is_file() and path != into / 'provenance.json'}


def verify_package(into):
    if into.is_symlink() or any(path.is_symlink() for path in into.rglob('*')):
        fail('package provenance contains a symlink')
    ledger = json.loads((into / 'provenance.json').read_text())
    if ledger.get('schema') != 1 or ledger.get('files') != file_hashes(into):
        fail('package files differ from their recorded provenance')
    original = into / 'System.app'
    if (ledger.get('package_sha256') != hashlib.sha256(original.read_bytes()).hexdigest()
            or ledger.get('package_bytes') != original.stat().st_size
            or ledger.get('identity') != package_identity(into)):
        fail('original package identity differs from its recorded provenance')
    return ledger


def publish(app, version, url, entry, output):
    if not version or any(not part.isdigit() for part in version.split('.')):
        fail('BC_VERSION is not a dotted numeric version')
    digest = hashlib.sha256(app).hexdigest()
    destination = output / version / digest
    if destination.exists() or destination.is_symlink():
        ledger = verify_package(destination)
        if ledger.get('package_sha256') != digest:
            fail('existing package bytes differ from their content-addressed destination')
        if (ledger.get('bc_version'), ledger.get('source_url'), ledger.get('source_entry')) != (
                version, url, entry):
            fail('existing package provenance names a different source')
        return destination
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='symbols-', dir=destination.parent) as folder:
        staged = pathlib.Path(folder) / 'package'
        unpack(app, staged)
        identity = package_identity(staged)
        (staged / 'System.app').write_bytes(app)
        ledger = {'schema': 1, 'bc_version': version, 'source_url': url, 'source_entry': entry,
                  'package_sha256': digest, 'package_bytes': len(app), 'identity': identity,
                  'files': file_hashes(staged)}
        (staged / 'provenance.json').write_text(json.dumps(ledger, indent=2, sort_keys=True) + '\n')
        verify_package(staged)
        if destination.exists() or destination.is_symlink():
            fail('package destination appeared during publication; no files replaced')
        staged.rename(destination)
    return destination


def artifact_version(value):
    if re.fullmatch(r'[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+', value) is None:
        raise argparse.ArgumentTypeError('version must contain four numeric components')
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=pathlib.Path, default=OUT,
                        help='parent of version/hash package directories (default: build/symbols)')
    operation = parser.add_mutually_exclusive_group()
    operation.add_argument('--version', type=artifact_version,
                           help='explicit public OnPrem platform version; BC_VERSION stays unchanged')
    operation.add_argument("--verify", type=pathlib.Path,
                        help="verify an existing package without downloading or writing")
    arguments = parser.parse_args()
    if arguments.verify is not None:
        ledger = verify_package(arguments.verify)
        print(json.dumps({key: ledger[key] for key in
                          ('package_sha256', 'package_bytes', 'identity')}, sort_keys=True))
        return
    version = arguments.version or (ROOT / "BC_VERSION").read_text().strip()
    url = f"{CDN}/onprem/{version}/platform"
    total = size_of(url)
    print(f"symbols: {url} is {total / 1e9:.2f} GB, and this reads three ranges of it")
    entries, directory = central_directory(url, total)
    name, local, compressed = entry_named(directory, entries, "system.app")
    print(f"symbols: {name} at {local}, {compressed} bytes compressed")
    app = member(url, local, compressed)
    digest = hashlib.sha256(app).hexdigest()
    print(f"symbols: {len(app)} bytes, SHA-256 {digest}")
    destination = publish(app, version, url, name, arguments.output)
    declarations = len(list((destination / 'src').rglob('*.al')))
    print(f"symbols: {declarations} AL files; original NAVX and provenance under {destination}")
    print(f"symbols: AGIRU_SYSTEM_SYMBOLS={destination}")


if __name__ == "__main__":
    main()
