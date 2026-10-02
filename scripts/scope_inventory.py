#!/usr/bin/env python3
"""Inventory AL source independently; namespace selection is diagnostic, never a filter."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import subprocess


KINDS = frozenset(('table', 'tableextension', 'page', 'pageextension', 'pagecustomization',
                   'codeunit', 'report', 'reportextension', 'query', 'xmlport', 'enum',
                   'enumextension', 'interface', 'permissionset', 'permissionsetextension',
                   'profile', 'profileextension', 'entitlement', 'controladdin', 'dotnet'))
LEXEME = re.compile(r'#[^\n]*|//[^\n]*|/\*[\s\S]*?\*/|'
                    r'\'(?:\'\'|[^\'])*\'|"(?:""|[^"])*"|'
                    r'[^\W\d]\w*|[0-9]+|[\s\ufeff]+|[\s\S]')
DIRECTIVES = frozenset(('#if', '#elif', '#else', '#endif', '#define', '#undef', '#pragma',
                        '#region', '#endregion'))


def tokens(text):
    result = []
    for match in LEXEME.finditer(text):
        value = match[0]
        if all(char.isspace() or char == '\ufeff' for char in value) or value.startswith(('//', '/*')):
            continue
        if value.lstrip(' \t\ufeff').startswith('#'):
            word = re.match(r'#\s*([A-Za-z]+)', value.lstrip(' \t\ufeff'))
            directive = '#' + word[1].lower() if word else '#'
            if directive not in DIRECTIVES:
                raise ValueError(f'unknown AL preprocessor directive {directive!r}')
            result.append(directive)
            continue
        if value in ("'", '"') or (value == '/' and text[match.start():].startswith('/*')):
            raise ValueError('unterminated AL comment or string')
        if not value.startswith("'"):
            result.append(value)
    return result


def identifier(value):
    if value.startswith('"') and value.endswith('"'):
        return value[1:-1].replace('""', '"')
    if re.fullmatch(r'[^\W\d]\w*', value):
        return value
    raise ValueError(f'expected an AL identifier, got {value!r}')


class Depth:
    def __init__(self, opening, ending):
        self.opening, self.ending = opening, ending
        self.value, self.branches = 0, []

    def step(self, token):
        if token == '#if':
            self.branches.append((self.value, []))
        elif token in ('#else', '#elif'):
            if not self.branches:
                raise ValueError('conditional branch starts before the object boundary')
            start, ends = self.branches[-1]
            ends.append(self.value)
            self.value = start
        elif token == '#endif':
            if not self.branches:
                raise ValueError('conditional branch starts before the object boundary')
            _, ends = self.branches.pop()
            ends.append(self.value)
            if len(set(ends)) != 1:
                raise ValueError('conditional brace depths require a variant-specific inventory')
        elif token == self.opening:
            self.value += 1
        elif token == self.ending:
            self.value -= 1
        return self.value


def closing(values, start, opening, ending):
    depth = Depth(opening, ending)
    for at in range(start, len(values)):
        if depth.step(values[at]) == 0 and values[at] == ending:
            return at
    raise ValueError(f'unclosed {opening!r}')


def test_methods(body):
    depth = Depth('{', '}')
    attributes, pending = 0, 0
    methods = []
    subtype = False
    for at, value in enumerate(body):
        if value in ('#if', '#else', '#elif') and pending:
            raise ValueError('conditional test headers require a variant-specific inventory')
        if depth.step(value) != 1:
            continue
        if value.lower() == 'subtype' and [part.lower() for part in body[at:at + 4]] == [
                'subtype', '=', 'test', ';']:
            subtype = True
        if value == '[' and [part.lower() for part in body[at:at + 3]] == ['[', 'test', ']']:
            attributes += 1
            pending += 1
        if value.lower() == 'procedure' and pending:
            if pending != 1 or at + 2 >= len(body) or body[at + 2] != '(':
                raise ValueError('Test attributes cannot be reconciled with procedures')
            methods.append(identifier(body[at + 1]))
            pending = 0
    if pending or attributes != len(methods):
        raise ValueError('Test attributes cannot be reconciled with procedures')
    return subtype, methods, attributes


def declarations(text, values=None):
    values = tokens(text) if values is None else values
    at, namespace = 0, ''
    objects = []
    while at < len(values):
        kind = values[at].lower()
        if kind in DIRECTIVES:
            at += 1
            continue
        if kind in ('namespace', 'using'):
            end = values.index(';', at)
            parts = values[at + 1:end]
            if not parts or any(part != '.' and identifier(part) != part for part in parts):
                raise ValueError('invalid namespace/using declaration')
            if kind == 'namespace':
                if namespace or objects:
                    raise ValueError('namespace must precede all objects and occur once')
                namespace = ''.join(parts)
            at = end + 1
            continue
        if values[at] == '[':
            at = closing(values, at, '[', ']') + 1
            continue
        if kind not in KINDS:
            raise ValueError(f'unrecognized top-level AL token {values[at]!r}')
        at += 1
        number, name = None, ''
        if kind != 'dotnet':
            if at < len(values) and values[at].isascii() and values[at].isdecimal():
                number = int(values[at])
                at += 1
            if at >= len(values):
                raise ValueError('object has no name/body')
            name = identifier(values[at])
            at += 1
        start = values.index('{', at)
        headers = [(kind, number, name)]
        header_at = at
        while header_at < start:
            variant_kind = values[header_at].lower()
            if variant_kind not in KINDS:
                header_at += 1
                continue
            header_at += 1
            variant_number = None
            if values[header_at].isascii() and values[header_at].isdecimal():
                variant_number = int(values[header_at])
                header_at += 1
            variant_name = identifier(values[header_at])
            headers.append((variant_kind, variant_number, variant_name))
            header_at += 1
        end = closing(values, start, '{', '}')
        subtype, methods, attributes = test_methods(values[start:end + 1])
        for header_kind, header_number, header_name in headers:
            objects.append({'kind': header_kind, 'id': header_number, 'name': header_name,
                            'namespace': namespace, 'test_subtype': subtype, 'methods': methods,
                            'test_attributes': attributes, 'shared_body': len(headers) > 1})
        at = end + 1
    return namespace, objects


def namespace_selected(namespace, policy):
    if not namespace:
        return True
    name = namespace.lower()
    def longest(prefixes):
        return max((len(prefix) for prefix in prefixes if prefix and
                    (name == prefix.lower() or name.startswith(prefix.lower() + '.'))), default=0)
    return longest(policy['include']) > longest(policy['exclude'])


def configured_apps(configuration):
    apps = configuration['apps']
    if not apps or len({app['name'] for app in apps}) != len(apps):
        raise ValueError('configured app names must be present and unique')
    for app in apps:
        path = PurePosixPath(app['source'])
        if not app['name'] or path.is_absolute() or not path.parts or any(
                part in ('.', '..') for part in app['source'].split('/')) or '\\' in app['source']:
            raise ValueError('configured apps need explicit relative source roots')
    return apps


def product_rules(policy):
    rules = []
    seen = set()
    entries = policy.get('product_exclude', [])
    if not isinstance(entries, list):
        raise ValueError('product exclusions must be an array')
    for entry in entries:
        if not isinstance(entry, str):
            raise ValueError('product exclusion must be a reason:source string')
        reason, separator, source = entry.partition(':')
        if not separator or reason not in (
                'bc-licensing', 'microsoft-cloud', 'licensing-and-microsoft-cloud'):
            raise ValueError('product exclusion needs an approved reason and source')
        path = PurePosixPath(source)
        parts = source.rstrip('/').split('/')
        if not source or path.is_absolute() or '\\' in source or '//' in source or any(
                part in ('', '.', '..') for part in parts):
            raise ValueError('product exclusion needs a bounded relative source path')
        if source in seen:
            raise ValueError('duplicate product exclusion source')
        seen.add(source)
        rules.append((reason, source))
    return rules


def product_reason(source, rules):
    return next((reason for reason, selected in rules if source == selected or
                 (selected.endswith('/') and source.startswith(selected))), None)


def inventory(root, configuration, policy):
    root = root.resolve(strict=True)
    if not root.is_dir():
        raise ValueError('AL source root must be a directory')
    apps = configured_apps(configuration)
    rules = product_rules(policy)
    declared_roots = {path.parent.relative_to(root).as_posix()
                      for path in root.rglob('app.json') if path.is_file()}
    files, objects, errors = [], [], []
    kinds, encodings = Counter(), Counter()
    source_digest = hashlib.sha256()
    for path in sorted(root.rglob('*')):
        if not path.is_file() or path.suffix.lower() != '.al':
            continue
        source = path.relative_to(root).as_posix()
        parents = [parent.as_posix() for parent in PurePosixPath(source).parents]
        app_root = next((parent for parent in parents if parent in declared_roots), None)
        configured = [app['name'] for app in apps if
                      source.startswith(app['source'] + '/')]
        reason = product_reason(source, rules)
        row = {'source': source, 'app_root': app_root, 'configured_apps': configured,
               'product_exclusion_reason': reason}
        files.append(row)
        try:
            raw = path.read_bytes()
            row['sha256'] = hashlib.sha256(raw).hexdigest()
            source_digest.update(source.encode() + b'\0' + bytes.fromhex(row['sha256']))
            encoding = 'utf-16' if raw.startswith((b'\xff\xfe', b'\xfe\xff')) else 'utf-8-sig'
            encodings[encoding] += 1
            text = raw.decode(encoding)
            values = tokens(text)
            row['raw_test_attributes'] = sum(value == '[' and
                [part.lower() for part in values[at:at + 3]] == ['[', 'test', ']']
                for at, value in enumerate(values))
            namespace, found = declarations(text, values)
            row['namespace'] = namespace
            row['namespace_selected'] = namespace_selected(namespace, policy)
            row['objects'] = len(found)
            row['conditional_source'] = bool(re.search(r'^\s*#(?:if|elif)\b', text, re.M | re.I))
            for index, item in enumerate(found):
                item.update(source=source, declaration_index=index, app_root=app_root,
                            configured_apps=configured, namespace_selected=row['namespace_selected'],
                            product_exclusion_reason=reason)
                kinds[item['kind']] += 1
                objects.append(item)
        except (OSError, UnicodeError, ValueError) as error:
            row['error'] = str(error)
            errors.append({'source': source, 'error': str(error)})
    if not files:
        errors.append({'source': '.', 'error': 'no AL sources found'})
    missing_roots = [app['source'] for app in apps if not (root / app['source']).is_dir()]
    errors.extend({'source': source, 'error': 'configured app root is missing'}
                  for source in missing_roots)
    errors.extend({'source': source, 'error': 'product exclusion target is missing'}
                  for _, source in rules if not (root / source).exists())
    test_units = [item for item in objects if item['kind'] == 'codeunit' and item['test_subtype']]
    return {'format': 1, 'scope': 'raw source; conditional variants retained, not an executable manifest',
            'source_root': str(root), 'source_sha256': source_digest.hexdigest(),
            'summary': {'files': len(files), 'objects': len(objects),
                        'test_codeunits': len(test_units),
                        'test_methods': sum(len(item['methods']) for item in test_units),
                        'test_attributes': sum(item['test_attributes'] for item in test_units),
                        'raw_test_attributes': sum(row.get('raw_test_attributes', 0) for row in files),
                        'product_excluded_objects': sum(bool(item['product_exclusion_reason'])
                                                        for item in objects),
                        'product_excluded_test_methods': sum(len(item['methods']) for item in test_units
                                                            if item['product_exclusion_reason']),
                        'product_required_test_methods': sum(len(item['methods']) for item in test_units
                                                            if not item['product_exclusion_reason']),
                        'objects_by_kind': dict(sorted(kinds.items())),
                        'encodings': dict(sorted(encodings.items())),
                        'outside_configured_app_files': sum(not row['configured_apps'] for row in files),
                        'unmeasured_files': len([row for row in files if 'error' in row])},
            'configured_apps': apps, 'declared_app_roots': sorted(declared_roots),
            'files': files, 'objects': objects, 'errors': errors}


def main():
    repository = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('root', type=Path)
    parser.add_argument('--apps', type=Path, default=repository / 'apps.json')
    parser.add_argument('--scope', type=Path, default=repository / 'scope.json')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    apps_bytes, scope_bytes = args.apps.read_bytes(), args.scope.read_bytes()
    report = inventory(args.root, json.loads(apps_bytes), json.loads(scope_bytes))
    revision = subprocess.run(['git', '-C', str(args.root), 'rev-parse', 'HEAD'],
                              capture_output=True, text=True, check=False)
    report['source_revision'] = revision.stdout.strip() if revision.returncode == 0 else None
    report['apps_sha256'] = hashlib.sha256(apps_bytes).hexdigest()
    report['scope_sha256'] = hashlib.sha256(scope_bytes).hexdigest()
    report['namespace_policy'] = json.loads(scope_bytes)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'summary': report['summary'], 'errors': report['errors'],
                      'output': str(args.output)}))
    return int(bool(report['errors']))


if __name__ == '__main__':
    raise SystemExit(main())
