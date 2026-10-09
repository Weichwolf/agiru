#!/usr/bin/env python3
"""Inventory AL source independently; namespace selection is diagnostic, never a filter."""
import argparse
from collections import Counter
import hashlib
from itertools import product
import json
from pathlib import Path, PurePosixPath
import re

from source_revision import bc_revision


KINDS = frozenset(('table', 'tableextension', 'page', 'pageextension', 'pagecustomization',
                   'codeunit', 'report', 'reportextension', 'query', 'xmlport', 'enum',
                   'enumextension', 'interface', 'permissionset', 'permissionsetextension',
                   'profile', 'profileextension', 'entitlement', 'controladdin', 'dotnet'))
LEXEME = re.compile(r'#[^\n]*|//[^\n]*|/\*[\s\S]*?\*/|'
                    r'\'(?:\'\'|[^\'])*\'|"(?:""|[^"])*"|'
                    r'[^\W\d]\w*|[0-9]+|[\s\ufeff]+|[\s\S]')
DIRECTIVES = frozenset(('#if', '#elif', '#else', '#endif', '#define', '#undef', '#pragma',
                        '#region', '#endregion'))
# The fallback visits at most 256 symbol assignments; exceeding this work budget refuses
# the file explicitly rather than omitting conditional declarations or methods.
MAX_VARIANT_SYMBOLS = 8


def tokens(text, origins=None, conditions=None):
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
            if conditions is not None:
                conditions[len(result)] = value[word.end():].split('//', 1)[0].strip()
            result.append(directive)
            if origins is not None:
                origins.append(match.start())
            continue
        if value in ("'", '"') or (value == '/' and text[match.start():].startswith('/*')):
            raise ValueError('unterminated AL comment or string')
        if not value.startswith("'"):
            result.append(value)
            if origins is not None:
                origins.append(match.start())
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
            self.branches.append([self.value, [], False])
        elif token in ('#else', '#elif'):
            if not self.branches:
                raise ValueError('conditional branch starts before the object boundary')
            start, ends, _ = self.branches[-1]
            ends.append(self.value)
            self.value = start
            if token == '#else':
                self.branches[-1][2] = True
        elif token == '#endif':
            if not self.branches:
                raise ValueError('conditional branch starts before the object boundary')
            start, ends, has_else = self.branches.pop()
            ends.append(self.value)
            if not has_else:
                ends.append(start)
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


def test_methods(body, origins=None, locations=None):
    depth = Depth('{', '}')
    attributes, pending = 0, 0
    methods = []
    subtype = False
    attribute_origin = None
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
            attribute_origin = origins[at] if origins is not None else None
        if value.lower() == 'procedure' and pending:
            if pending != 1 or at + 2 >= len(body) or body[at + 2] != '(':
                raise ValueError('Test attributes cannot be reconciled with procedures')
            methods.append(identifier(body[at + 1]))
            if locations is not None:
                locations.append((attribute_origin, (origins[at], methods[-1])))
            pending = 0
    if pending or attributes != len(methods):
        raise ValueError('Test attributes cannot be reconciled with procedures')
    return subtype, methods, attributes


def raw_declarations(text, values=None, origins=None):
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
        declaration_origin = origins[at] if origins is not None else None
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
        locations = [] if origins is not None else None
        subtype, methods, attributes = test_methods(values[start:end + 1],
            origins[start:end + 1] if origins is not None else None, locations)
        for header_kind, header_number, header_name in headers:
            objects.append({'kind': header_kind, 'id': header_number, 'name': header_name,
                            'namespace': namespace, 'test_subtype': subtype, 'methods': methods,
                            'test_attributes': attributes, 'shared_body': len(headers) > 1})
            if origins is not None:
                objects[-1].update(_origin=declaration_origin, _methods=locations)
        at = end + 1
    return namespace, objects


def condition(expression, enabled):
    parts = re.findall(r'[^\W\d]\w*|[()]|[^\s]', expression)
    at = 0

    def unary():
        nonlocal at
        if at >= len(parts):
            raise ValueError('incomplete conditional expression')
        value = parts[at]
        at += 1
        if value.lower() == 'not':
            return not unary()
        if value == '(':
            result = binary('or')
            if at >= len(parts) or parts[at] != ')':
                raise ValueError('unclosed conditional expression')
            at += 1
            return result
        if not re.fullmatch(r'[^\W\d]\w*', value) or value.lower() in ('and', 'or'):
            raise ValueError(f'invalid conditional operand {value!r}')
        return value in enabled

    def binary(operator):
        nonlocal at
        operand = unary if operator == 'and' else lambda: binary('and')
        result = operand()
        while at < len(parts) and parts[at].lower() == operator:
            at += 1
            right = operand()
            result = (result and right) if operator == 'and' else (result or right)
        return result

    result = binary('or')
    if at != len(parts):
        raise ValueError('unconsumed conditional expression')
    return result


def conditional_indices(values, conditions, enabled):
    enabled = set(enabled)
    active, stack, selected = True, [], []
    for at, value in enumerate(values):
        if value == '#if':
            branch = condition(conditions[at], enabled)
            stack.append([active, branch, False])
            active = active and branch
        elif value in ('#elif', '#else'):
            if not stack or stack[-1][2]:
                raise ValueError('invalid conditional branch order')
            parent, taken, _ = stack[-1]
            branch = condition(conditions[at], enabled) if value == '#elif' else True
            active = parent and not taken and branch
            stack[-1][1] = taken or branch
            stack[-1][2] = value == '#else'
        elif value == '#endif':
            if not stack:
                raise ValueError('conditional end has no opening')
            active = stack.pop()[0]
        elif value in ('#define', '#undef'):
            name = identifier(conditions[at])
            if active:
                enabled.add(name) if value == '#define' else enabled.discard(name)
        elif active and value not in DIRECTIVES:
            selected.append(at)
    if stack:
        raise ValueError('unclosed conditional directive')
    return selected


def variant_declarations(text, refusals=None):
    origins, conditions = [], {}
    values = tokens(text, origins, conditions)
    symbols = sorted({part for at, expression in conditions.items()
        if values[at] in ('#if', '#elif')
        for part in re.findall(r'[^\W\d]\w*', expression)
        if part.lower() not in ('and', 'or', 'not')})
    if len(symbols) > MAX_VARIANT_SYMBOLS:
        raise ValueError(f'conditional inventory exceeds {MAX_VARIANT_SYMBOLS}-symbol work budget')
    merged, namespaces, witnessed = {}, set(), set()
    for flags in product((False, True), repeat=len(symbols)):
        assignment = dict(zip(symbols, flags))
        enabled = [name for name, flag in assignment.items() if flag]
        selected = conditional_indices(values, conditions, enabled)
        try:
            namespace, objects = raw_declarations(text, [values[at] for at in selected],
                                                 [origins[at] for at in selected])
        except ValueError as error:
            if refusals is None:
                raise
            refusals.append({'symbols': assignment, 'error': str(error)})
            continue
        namespaces.add(namespace)
        for item in objects:
            key = (item.pop('_origin'), item['kind'], item['id'], item['name'], item['namespace'])
            locations = item.pop('_methods')
            witnessed.update(origin for origin, _ in locations)
            if key not in merged:
                item.update(_locations={}, conditional_variants=[])
                merged[key] = item
            held = merged[key]
            if held['test_subtype'] != item['test_subtype']:
                raise ValueError('conditional Test subtype requires separate build populations')
            for origin, method in locations:
                if origin in held['_locations'] and held['_locations'][origin] != method:
                    raise ValueError('conditional test headers require separate build populations')
                held['_locations'][origin] = method
            held['conditional_variants'].append(assignment)
    expected = {origins[at] for at, value in enumerate(values) if value == '[' and
                [part.lower() for part in values[at:at + 3]] == ['[', 'test', ']']}
    if witnessed != expected:
        raise ValueError('conditional inventory cannot reconcile every raw Test attribute')
    if not merged:
        raise ValueError('no conditional variant has measurable declarations')
    objects = []
    for key, item in sorted(merged.items(), key=lambda pair: (pair[0][0], repr(pair[0][1:]))):
        locations = item.pop('_locations')
        item['methods'] = [method[1] for _, method in sorted(locations.items())]
        item['test_attributes'] = len(locations)
        objects.append(item)
    return (next(iter(namespaces)) if len(namespaces) == 1 else ''), objects


def declarations(text, values=None, refusals=None):
    try:
        return raw_declarations(text, values)
    except ValueError as error:
        if str(error) != 'conditional brace depths require a variant-specific inventory':
            raise
        return variant_declarations(text, refusals)


def namespace_selected(namespace, policy):
    if not namespace:
        return True
    name = namespace.lower()
    def longest(prefixes):
        return max((len(prefix) for prefix in prefixes if prefix and
                    (name == prefix.lower() or name.startswith(prefix.lower() + '.'))), default=0)
    return longest(policy['include']) > longest(policy['exclude'])


def area_selected(area, policy):
    name = area.lower()
    return name not in {value.lower() for value in policy.get('area_exclude', [])} and not any(
        name.endswith(value.lower()) for value in policy.get('area_exclude_suffix', []))


def source_includes(policy):
    entries = policy.get('source_include', [])
    if not isinstance(entries, list):
        raise ValueError('source includes must be an array')
    seen = set()
    for source in entries:
        if not isinstance(source, str) or not source.endswith('.al'):
            raise ValueError('source includes require exact AL file identities')
        path = PurePosixPath(source)
        if path.is_absolute() or '\\' in source or any(
                part in ('', '.', '..') for part in source.split('/')):
            raise ValueError('source includes need bounded relative file paths')
        if source in seen:
            raise ValueError('duplicate source include')
        seen.add(source)
    return seen


def selection_reason(namespace, area, test, policy, source_domain='bcapps', source=None):
    if source_domain == 'system-symbols':
        return None
    if source_domain != 'bcapps':
        raise ValueError('unknown source domain')
    if source in policy.get('source_include', []):
        return None
    if not namespace_selected(namespace, policy):
        return 'selection-namespace'
    if test and not area_selected(area, policy):
        return 'selection-area'
    return None


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
        if source == 'system-symbols/':
            raise ValueError('System exclusions require a bounded source inside the package')
        if source in seen:
            raise ValueError('duplicate product exclusion source')
        seen.add(source)
        rules.append((reason, source))
    return rules


def product_reason(source, rules):
    return next((reason for reason, selected in rules if source == selected or
                 (selected.endswith('/') and source.startswith(selected))), None)


def inventory(root, configuration, policy, source_domain='bcapps'):
    root = root.resolve(strict=True)
    if not root.is_dir():
        raise ValueError('AL source root must be a directory')
    apps = configured_apps(configuration)
    all_rules = product_rules(policy)
    all_includes = source_includes(policy)
    if source_domain not in ('bcapps', 'system-symbols'):
        raise ValueError('unknown source domain')
    prefix = 'system-symbols/'
    rules = [(reason, source[len(prefix):] if source_domain == 'system-symbols' else source)
             for reason, source in all_rules
             if source.startswith(prefix) == (source_domain == 'system-symbols')]
    includes = [source[len(prefix):] if source_domain == 'system-symbols' else source
                for source in all_includes
                if source.startswith(prefix) == (source_domain == 'system-symbols')]
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
        area = next((source[len(app['source']) + 1:].split('/')[0] for app in apps
                     if source.startswith(app['source'] + '/')), '')
        reason = product_reason(source, rules)
        row = {'source': source, 'app_root': app_root, 'configured_apps': configured,
               'product_exclusion_reason': reason, 'area': area,
               'area_selected': area_selected(area, policy)}
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
            refusals = []
            namespace, found = declarations(text, values, refusals)
            if refusals:
                row['conditional_variant_refusals'] = refusals
                errors.append({'source': source,
                    'error': 'conditional build variants require classification',
                    'variants': refusals})
            row['namespace'] = namespace
            row['namespace_selected'] = namespace_selected(namespace, policy)
            row['objects'] = len(found)
            row['conditional_source'] = bool(re.search(r'^\s*#(?:if|elif)\b', text, re.M | re.I))
            for index, item in enumerate(found):
                omission = selection_reason(namespace, area,
                    item['kind'] == 'codeunit' and item['test_subtype'], policy, source_domain,
                    source if source_domain == 'bcapps' else prefix + source)
                selected = bool(configured) and reason is None and omission is None
                item.update(source=source, declaration_index=index, app_root=app_root,
                            configured_apps=configured, namespace_selected=row['namespace_selected'],
                            product_exclusion_reason=reason, area=area,
                            area_selected=row['area_selected'],
                            selection_selected=selected)
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
    errors.extend({'source': source, 'error': 'source include target is missing'}
                  for source in includes if not (root / source).is_file())
    test_units = [item for item in objects if item['kind'] == 'codeunit' and item['test_subtype']]
    return {'format': 1, 'scope': 'raw source; conditional variants retained, not an executable manifest',
            'source_root': str(root), 'source_domain': source_domain,
            'other_domain_rules': [{'reason': reason, 'source': source}
                                  for reason, source in all_rules
                                  if source.startswith(prefix) != (source_domain == 'system-symbols')],
            'source_sha256': source_digest.hexdigest(),
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
                        'selected_objects': sum(item['selection_selected'] for item in objects),
                        'selected_test_codeunits': sum(item['selection_selected'] for item in test_units),
                        'selected_test_methods': sum(len(item['methods']) for item in test_units
                                                     if item['selection_selected']),
                        'omitted_required_test_methods': sum(len(item['methods']) for item in test_units
                            if not item['selection_selected'] and not item['product_exclusion_reason']),
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
    parser.add_argument('--source-domain', choices=('bcapps', 'system-symbols'), default='bcapps')
    args = parser.parse_args()
    apps_bytes, scope_bytes = args.apps.read_bytes(), args.scope.read_bytes()
    report = inventory(args.root, json.loads(apps_bytes), json.loads(scope_bytes), args.source_domain)
    report['source_revision'] = bc_revision(args.root, repository)
    report['apps_sha256'] = hashlib.sha256(apps_bytes).hexdigest()
    report['scope_sha256'] = hashlib.sha256(scope_bytes).hexdigest()
    report['namespace_policy'] = json.loads(scope_bytes)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'summary': report['summary'], 'errors': report['errors'],
                      'output': str(args.output)}))
    return int(bool(report['errors']))


if __name__ == '__main__':
    raise SystemExit(main())
