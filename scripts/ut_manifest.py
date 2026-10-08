#!/usr/bin/env python3
"""Count the UT milestone from AL text, independently of the transpiler and binary."""
import argparse
import json
from pathlib import Path
import re

from scope_inventory import area_selected, declarations, namespace_selected, product_reason, product_rules

QUOTED = re.compile(r'"(?:""|[^"])*"')
IGNORED = re.compile(r'''"(?:""|[^"])*"|//[^\n]*|/\*.*?\*/|'(?:''|[^'])*'|\ufeff''', re.S)
OBJECT = re.compile(r'\bcodeunit\s+(\d+)\s+("(?:""|[^"])*"|[\w.]+)', re.I)
METHOD = re.compile(r'\[Test\]\s*(?:(?:\[[^\]]*\])\s*)*(?:(?:local|internal)\s+)?procedure\s+("(?:""|[^"])*"|\w+)\s*\(', re.I)


def identifier(value):
    return value[1:-1].replace('""', '"') if value.startswith('"') else value


def scan(root):
    entries = []
    for path in sorted(path for path in root.rglob('*')
                       if path.is_file() and path.suffix.lower() == '.al'):
        raw = path.read_bytes()
        encoding = 'utf-16' if raw.startswith((b'\xff\xfe', b'\xfe\xff')) else 'utf-8-sig'
        text = IGNORED.sub(lambda m: m[0] if m[0].startswith('"') else
                           '\n' * m[0].count('\n') + ' ', raw.decode(encoding))
        shape = QUOTED.sub(lambda m: '"' + re.sub(r'[^\n]', ' ', m[0][1:-1]) + '"', text)
        objects = list(OBJECT.finditer(shape))
        for at, obj in enumerate(objects):
            body = shape[obj.end():objects[at + 1].start() if at + 1 < len(objects) else len(shape)]
            name = identifier(text[obj.start(2):obj.end(2)])
            if not name.endswith((' UT', '-UT', '.UT')) or not re.search(r'\bSubtype\s*=\s*Test\s*;', body, re.I):
                continue
            methods = [identifier(text[obj.end() + m.start(1):obj.end() + m.end(1)])
                       for m in METHOD.finditer(body)]
            attributes = len(re.findall(r'\[Test\]', body, re.I))
            if not methods or len(methods) != attributes or len(methods) != len(set(methods)):
                raise ValueError(f'{path}: cannot reconcile {attributes} Test attributes with unique procedures')
            entries.append({'id': int(obj[1]), 'name': name, 'source': str(path), 'methods': methods})
    if not entries:
        raise ValueError(f'no UT codeunits found under {root}')
    if len({e['id'] for e in entries}) != len(entries) or len({e['name'] for e in entries}) != len(entries):
        raise ValueError('UT codeunit IDs and names must be unique')
    return sorted(entries, key=lambda e: e['id'])


def partition(entries, source_root, app_root, policy):
    """Keep raw identities while applying the transpiler's canonical selection policy."""
    if not policy.get('include'):
        raise ValueError('scope.json: the include list is empty')
    rules = product_rules(policy)
    for _, source in rules:
        if not source.startswith('system-symbols/') and not (source_root / source).exists():
            raise ValueError(f'scope.json: product exclusion target is missing: {source}')
    selected, excluded = [], []
    for entry in entries:
        path = Path(entry['source'])
        product = product_reason(path.relative_to(source_root).as_posix(), rules)
        raw = path.read_bytes()
        text = raw.decode('utf-16' if raw.startswith((b'\xff\xfe', b'\xfe\xff')) else 'utf-8-sig')
        namespace, _ = declarations(text)
        area = path.relative_to(app_root).parts[0].lower()
        reason = product
        if reason is None and namespace and not namespace_selected(namespace, policy):
            reason = 'selection-namespace'
        if reason is None and not area_selected(area, policy):
            reason = 'selection-area'
        if reason is None:
            selected.append(entry)
        else:
            excluded.append(dict(entry, reason=reason, product_exclusion_reason=product))
    return selected, excluded


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
