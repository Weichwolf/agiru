#!/usr/bin/env python3
"""Count the UT milestone from AL text, independently of the transpiler and binary."""
import argparse
import json
from pathlib import Path
import re

IGNORED = re.compile(r"//[^\n]*|/\*.*?\*/|'(?:''|[^'])*'", re.S)
OBJECT = re.compile(r'^\s*codeunit\s+(\d+)\s+("(?:""|[^"])*"|[\w.]+)', re.I | re.M)
METHOD = re.compile(r'\[Test\]\s*(?:(?:\[[^\]]*\])\s*)*(?:(?:local|internal)\s+)?procedure\s+("(?:""|[^"])*"|\w+)\s*\(', re.I)


def scan(root):
    entries = []
    for path in sorted(root.rglob('*.al')):
        raw = path.read_bytes()
        encoding = 'utf-16' if raw.startswith((b'\xff\xfe', b'\xfe\xff')) else 'utf-8-sig'
        text = IGNORED.sub(lambda m: '\n' * m.group().count('\n') + ' ', raw.decode(encoding))
        objects = list(OBJECT.finditer(text))
        for at, obj in enumerate(objects):
            body = text[obj.end():objects[at + 1].start() if at + 1 < len(objects) else len(text)]
            name = obj[2].strip('"').replace('""', '"')
            if not name.endswith((' UT', '-UT', '.UT')) or not re.search(r'\bSubtype\s*=\s*Test\s*;', body, re.I):
                continue
            methods = [m[1].strip('"').replace('""', '"') for m in METHOD.finditer(body)]
            attributes = len(re.findall(r'\[Test\]', body, re.I))
            if not methods or len(methods) != attributes or len(methods) != len(set(methods)):
                raise ValueError(f'{path}: cannot reconcile {attributes} Test attributes with unique procedures')
            entries.append({'id': int(obj[1]), 'name': name, 'source': str(path), 'methods': methods})
    if not entries:
        raise ValueError(f'no UT codeunits found under {root}')
    if len({e['id'] for e in entries}) != len(entries) or len({e['name'] for e in entries}) != len(entries):
        raise ValueError('UT codeunit IDs and names must be unique')
    return sorted(entries, key=lambda e: e['id'])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('root', type=Path)
    parser.add_argument('--tsv', action='store_true')
    args = parser.parse_args()
    entries = scan(args.root)
    if args.tsv:
        for entry in entries:
            print(f"{entry['id']}\t{entry['name']}\t{len(entry['methods'])}")
    else:
        print(json.dumps(entries, indent=2))
