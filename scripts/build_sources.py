#!/usr/bin/env python3
"""Project generated build inputs through the canonical policy, retaining raw identities."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import sys

from scope_inventory import configured_apps, product_reason, product_rules, selection_reason
from unity_groups import group_sources, slice_sources
from ut_manifest import IGNORED


def relative(value: str) -> str:
    path = PurePosixPath(value)
    if not value or path.is_absolute() or any(part in ('', '.', '..') for part in value.split('/')) \
            or any(character in value for character in '\\\r\n\t|;'):
        raise ValueError(f'invalid generated/source identity: {value!r}')
    return value


def read_json(path: Path):
    return json.loads(path.read_text())


def write_json(path: Path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + '.pending')
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n')
    temporary.replace(path)


def source_metadata(source: str, roots: dict[str, Path], apps: dict[str, str]):
    domain = 'system-symbols' if source.startswith('system-symbols/') else 'bcapps'
    name = source.removeprefix('system-symbols/') if domain == 'system-symbols' else source
    base = roots.get(domain)
    if base is None or not (base / name).is_file():
        return {'source': source, 'source_missing': True}
    raw = (base / name).read_bytes()
    text = raw.decode('utf-16' if raw.startswith((b'\xff\xfe', b'\xfe\xff')) else 'utf-8-sig')
    shape = IGNORED.sub(lambda match: '\n' * match[0].count('\n') + ' ', text)
    namespace = re.search(r'(?m)^\s*namespace\s+([\w.]+)\s*;', shape)
    owner = next((prefix for prefix in apps.values()
                  if name.startswith(prefix + '/')), '') if domain == 'bcapps' else ''
    area = name.removeprefix(owner + '/').split('/')[0] if owner else ''
    return {'source': source, 'source_missing': False,
            'source_sha256': hashlib.sha256(raw).hexdigest(),
            'namespace': namespace[1] if namespace else '', 'area': area,
            'test': name.lower().endswith('.codeunit.al') and
                    bool(re.search(r'\bSubtype\s*=\s*Test\s*;', shape, re.I))}


def record(arguments):
    root = arguments.root.resolve()
    generated = arguments.generated.resolve() if arguments.generated else root / 'apps'
    apps = {entry['name']: entry['source'] for entry in configured_apps(read_json(root / 'apps.json'))}
    previous = arguments.previous or generated / 'source-origins.json'
    document = read_json(previous) if previous.is_file() else {'schema': 1, 'sources': {}}
    if document.get('schema') != 1 or not isinstance(document.get('sources'), dict):
        raise ValueError('invalid generated source-origin manifest')
    origins = document['sources']
    for cpp, entry in origins.items():
        relative(cpp)
        relative(entry['source'])
    for path in sorted(generated.rglob('*.cpp')):
        cpp = relative(path.relative_to(generated).as_posix())
        with path.open() as stream:
            header = stream.readline()
        match = re.fullmatch(r'// Generated from (.+)\. Do not edit\.\n?', header)
        if match is None:
            continue
        source = relative(match[1])
        app = PurePosixPath(cpp).parts[0]
        if app == 'platform':
            source = 'system-symbols/' + source
        elif app in apps:
            source = apps[app] + '/' + source
        else:
            raise ValueError(f'generated source has no configured app owner: {cpp}')
        origins[cpp] = {'source': relative(source)}
    roots = {'bcapps': arguments.bc_source.resolve()}
    if arguments.symbols:
        roots['system-symbols'] = arguments.symbols.resolve()
    metadata = {}
    for cpp, entry in origins.items():
        source = entry['source']
        if source not in metadata:
            metadata[source] = source_metadata(source, roots, apps)
        origins[cpp] = metadata[source]
    write_json(arguments.output or generated / 'source-origins.json',
               {'schema': 1, 'sources': origins})


def generation(generated: Path, scope_bytes: bytes, apps_bytes: bytes):
    source_list = generated / 'generation-sources.txt'
    scope = generated / 'generation-scope.json'
    apps = generated / 'generation-apps.json'
    if not source_list.is_file() or not scope.is_file() or not apps.is_file():
        return set(), False, 'current generation manifest is missing'
    sources = [relative(value) for value in source_list.read_text().splitlines() if value]
    if len(sources) != len(set(sources)):
        raise ValueError('current generation contains duplicate source identities')
    if any(not source.endswith('.cpp') for source in sources):
        raise ValueError('current generation contains a non-C++ source identity')
    current = scope.read_bytes() == scope_bytes and apps.read_bytes() == apps_bytes
    return set(sources), current, '' if current else 'generation policy/app roots differ from current configuration'


def project(arguments):
    root = arguments.root.resolve()
    generated = root / 'apps'
    scope_bytes = (root / 'scope.json').read_bytes()
    policy = json.loads(scope_bytes)
    if not policy.get('include'):
        raise ValueError('scope.json: the include list is empty')
    rules = product_rules(policy)
    apps_bytes = (root / 'apps.json').read_bytes()
    configured_apps(json.loads(apps_bytes))
    emitted, current, generation_error = generation(generated, scope_bytes, apps_bytes)
    manifest = generated / 'source-origins.json'
    origins = read_json(manifest) if manifest.is_file() else {'schema': 1, 'sources': {}}
    if origins.get('schema') != 1 or not isinstance(origins.get('sources'), dict):
        raise ValueError('invalid generated source-origin manifest')
    origins = origins['sources']
    if arguments.slice:
        raw = slice_sources(arguments.slice)
    else:
        app = relative(arguments.app)
        if '/' in app:
            raise ValueError('an app name is not a path')
        raw = sorted({path.relative_to(generated).as_posix()
                      for path in (generated / app).rglob('*.cpp')} |
                     {path for path in emitted | origins.keys() if path.startswith(app + '/')})
    raw = [relative(value) for value in raw]
    groups = group_sources(raw)
    rows, selected, errors = [], [], []
    if not current:
        errors.append(generation_error)
    for cpp in raw:
        origin = origins.get(cpp, {})
        source = origin.get('source')
        if source:
            relative(source)
        reason = product_reason(source, rules) if source else None
        decision = 'product-excluded' if reason else 'selected'
        if not reason and source and not origin.get('source_missing', True):
            domain = 'system-symbols' if source.startswith('system-symbols/') else 'bcapps'
            reason = selection_reason(origin.get('namespace', ''), origin.get('area', ''),
                                      origin.get('test', False), policy, domain)
            if reason:
                decision = 'omitted'
        present = (generated / cpp).is_file()
        row = {'cpp': cpp, 'source': source, 'decision': decision, 'reason': reason,
               'present': present, 'emitted': cpp in emitted}
        rows.append(row)
        if decision != 'selected':
            if current and cpp in emitted:
                errors.append(f'current generation emitted a {decision} source: {cpp}')
            continue
        selected.append(cpp)
        if not source or origin.get('source_missing', True):
            errors.append(f'unverified original source: {cpp}')
        if current and cpp not in emitted:
            errors.append(f'required source was not emitted in the current generation: {cpp}')
        if not present:
            errors.append(f'missing {cpp}')
    if arguments.slice and not selected:
        errors.append('empty selected source population')
    receipt = {'schema': 1, 'population': 'slice' if arguments.slice else arguments.app,
               'scope_sha256': hashlib.sha256(scope_bytes).hexdigest(), 'generation_current': current,
               'raw': len(raw), 'selected': len(selected),
               'product_excluded': sum(row['decision'] == 'product-excluded' for row in rows),
               'omitted': sum(row['decision'] == 'omitted' for row in rows),
               'sources': rows, 'errors': errors}
    if arguments.receipt:
        write_json(arguments.receipt, receipt)
    if not arguments.check:
        for cpp in selected:
            print(f'{cpp}|{groups[cpp]}' if arguments.slice else cpp)
    else:
        print('build sources: ' + ', '.join(f'{key}={receipt[key]}' for key in
              ('raw', 'selected', 'product_excluded', 'omitted')) + f', errors={len(errors)}')
    if errors and not arguments.configure:
        for error in errors:
            print(f'build sources: {error}', file=sys.stderr)
        return 1
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    recorder = commands.add_parser('record')
    recorder.add_argument('--root', type=Path, required=True)
    recorder.add_argument('--generated', type=Path)
    recorder.add_argument('--previous', type=Path)
    recorder.add_argument('--output', type=Path)
    recorder.add_argument('--bc-source', type=Path, required=True)
    recorder.add_argument('--symbols', type=Path)
    projector = commands.add_parser('project')
    projector.add_argument('--root', type=Path, required=True)
    population = projector.add_mutually_exclusive_group(required=True)
    population.add_argument('--slice', type=Path)
    population.add_argument('--app')
    projector.add_argument('--receipt', type=Path)
    projector.add_argument('--configure', action='store_true')
    projector.add_argument('--check', action='store_true')
    arguments = parser.parse_args()
    try:
        if arguments.command == 'record':
            record(arguments)
            return 0
        return project(arguments)
    except (OSError, UnicodeError, ValueError, KeyError, TypeError) as error:
        print(f'build sources: {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
