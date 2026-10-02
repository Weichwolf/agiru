from pathlib import Path
import difflib
import hashlib
import importlib.util
import json
import re
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
identity = json.loads((root / 'source-identity.json').read_text())
origin = Path(identity['origin'])

def module(name, relative):
    spec = importlib.util.spec_from_file_location(name, source / relative)
    loaded = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(loaded)
    return loaded

verify = module('verify_snapshot', 'scripts/verify_snapshot.py')
manifest = module('ut_manifest', 'scripts/ut_manifest.py')
if verify.digest(origin) != identity['base_source_sha256']:
    raise RuntimeError('predecessor source image changed')
paths = ['src/gen/CodeunitWriter.h', 'src/gen/CodeunitWriter.cpp',
         'src/gen/TableWriter.cpp', 'src/gen/PageWriter.h', 'src/gen/PageWriter.cpp',
         'src/gen/BodyWriter.cpp', 'src/tc/Main.cpp', 'test/toolchain.py', 'test/slice']
identity.update(source_sha256=verify.digest(source), base_digest_reverified_unchanged=True,
                production_integrated=False, prototype_paths=paths)
(root / 'source-identity.json').write_text(json.dumps(identity, indent=2) + '\n')

def save(name, value):
    (artifacts / f'{name}.json').write_text(json.dumps(value, indent=2) + '\n')

def image(folder):
    return {str(path.relative_to(folder)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in folder.rglob('*') if path.is_file()}

old, new = image(origin / 'apps'), image(source / 'apps')
generated = {'before_files': len(old), 'after_files': len(new),
             'missing': sorted(old.keys() - new.keys()), 'added': sorted(new.keys() - old.keys()),
             'changed': sorted(name for name in old.keys() & new.keys() if old[name] != new[name]),
             'bcapps_main': subprocess.check_output(['git', '-C', '/home/cosmo/Git/BCApps',
                                                     'rev-parse', 'main'], text=True).strip(),
             'translation_exit': json.loads((artifacts / 'transpile.json').read_text())['exit'],
             'complete_translation': True, 'declaration_coverage_proved': False,
             'full_al_compilation_proved': False,
             'inherited_image_is_not_successful_regeneration': False}
save('generated-image', generated)
renamed = [
    ('base/system/test_tools/code_coverage/xmlport/CodeCoverageDetailed',
     'base/system/test_tools/code_coverage/xmlport/CodeCoverageDetailed_9991', 'PageId', 9991),
    ('system/system/security/access_control/table/PermissionSetBuffer',
     'system/system/security/access_control/table/PermissionSetBuffer_9862', 'TableId', 9862),
    ('tests/core/table/TestTableC', 'tests/core/table/TestTableC_132512', 'TableId', 132512),
]
renames = {old + suffix: new + suffix
           for old, new, _, _ in renamed for suffix in ('.cpp', '.def.cpp')}
appended = [stem + suffix for stem in (
    'base/system/security/access_control/table/PermissionSetBuffer_9009',
    'test_runner/system/test_tools/code_coverage/xmlport/CodeCoverageDetailed_130471',
    'tests/core/table/TestTableC_139063') for suffix in ('.cpp', '.def.cpp')]
old_slice = (origin / 'test/slice').read_text().splitlines()
new_slice = (source / 'test/slice').read_text().splitlines()
if new_slice != [renames.get(line, line) for line in old_slice] + appended:
    raise RuntimeError('slice order or unrelated entries changed')
for before_stem, after_stem, kind, identifier in renamed:
    declaration = f'{kind} kId{{{identifier}}}'
    if (declaration not in (origin / 'apps' / (before_stem + '.h')).read_text() or
            declaration not in (source / 'apps' / (after_stem + '.h')).read_text()):
        raise RuntimeError('a slice rename changed original AL identity')
def active(lines):
    return [line for line in lines if line and not line.startswith('#')]
missing_old = [line for line in active(old_slice) if not (source / 'apps' / line).is_file()]
missing_new = [line for line in active(new_slice) if not (source / 'apps' / line).is_file()]
if sorted(missing_old) != sorted(renames) or missing_new:
    raise RuntimeError('slice path closure differs from reviewed renames')
save('slice-identity', {'before': len(active(old_slice)), 'after': len(active(new_slice)),
                        'renamed': renames, 'appended': appended,
                        'old_manifest_missing_current_outputs': missing_old,
                        'current_missing_outputs': missing_new,
                        'unrelated_order_and_entries_unchanged': True,
                        'original_ids_preserved': True, 'full_slice_compilation_proved': False})
before = json.loads((Path('/home/cosmo/Git/agiru/build/verify/20260930T210740Z-797012') /
                     'artifacts/ut.log.manifest.json').read_text())
after = manifest.scan(Path('/home/cosmo/Git/BCApps/src/Layers/W1/Tests'))

def population(entries):
    return {(entry['id'], entry['name'], method)
            for entry in entries for method in entry['methods']}

previous, current = population(before), population(after)
ut = {'before_codeunits': len(before), 'after_codeunits': len(after),
      'before_methods': len(previous), 'after_methods': len(current),
      'missing': sorted(previous - current), 'added': sorted(current - previous),
      'execution_proof': False}
save('ut-population', ut)
patch = []
for name in paths:
    patch.extend(difflib.unified_diff((origin / name).read_text().splitlines(keepends=True),
                                    (source / name).read_text().splitlines(keepends=True),
                                    fromfile='before/' + name, tofile='after/' + name))
(artifacts / 'prototype.patch').write_text(''.join(patch))

def failures(log):
    units = set()
    for line in log.read_text().splitlines():
        match = re.search(r'FAIL\s+.*?/test/gate/([^/:]+)\.cpp:', line)
        escaped = re.search(r'FAIL\s+an (?:unknown )?exception left (\S+)', line)
        if match:
            units.add(match[1].removesuffix('Gate'))
        elif escaped:
            units.add(escaped[1])
    return units

comparison = []
for tag in ('clang', 'gcc'):
    old_failures = failures(origin.parent / 'artifacts' / f'test-{tag}.log')
    new_failures = failures(artifacts / f'test-{tag}.log')
    comparison.append({'compiler': tag, 'before_failed_cases': len(old_failures),
                       'after_failed_cases': len(new_failures),
                       'added_failures': sorted(new_failures - old_failures),
                       'removed_failures': sorted(old_failures - new_failures)})
save('local-failure-comparison', comparison)

def units(path):
    result = {}
    current_unit = None
    for line in path.read_text().splitlines():
        match = re.match(r'^== (.+?) \(exit \d+\) ==$', line)
        if match:
            current_unit = match[1]
            result[current_unit] = set()
        match = re.match(r'^.+?/(include|src|test)/(.+?):\d+:\d+: (?:error|warning): (.+)$', line)
        if match and current_unit is not None:
            result[current_unit].add((match[1] + '/' + match[2], match[3]))
    if not result:
        raise RuntimeError('missing analyzed units: ' + str(path))
    return result

before_units = {}
for tag in ('main', 'table', 'body'):
    before_units.update(units(origin.parent / f'artifacts/lint-{tag}-targeted.log'))
baseline = root.parent / 'native-contract-20261001'
for tag in ('codeunit', 'page'):
    name = 'src/gen/' + ('CodeunitWriter.cpp' if tag == 'codeunit' else 'PageWriter.cpp')
    if (baseline / 'source' / name).read_bytes() != (origin / name).read_bytes():
        raise RuntimeError('analysis baseline source changed: ' + name)
    before_units.update(units(baseline / f'artifacts/{tag}-writer-targeted-final.log'))
selected = {'src/tc/Main.cpp': 'main', 'src/gen/CodeunitWriter.cpp': 'codeunit',
            'src/gen/TableWriter.cpp': 'table', 'src/gen/BodyWriter.cpp': 'body',
            'src/gen/PageWriter.cpp': 'page'}
before_findings = set().union(*(before_units[name] for name in selected))
after_findings = set().union(*(units(artifacts / f'lint-{tag}-targeted.log')[name]
                              for name, tag in selected.items()))
def canonical(findings):
    return {(path, re.sub(r'has cognitive complexity of \d+', 'has cognitive complexity', message))
            for path, message in findings}

def complexities(findings):
    result = {}
    for path, message in findings:
        match = re.search(r"function '(.+)' has cognitive complexity of (\d+)", message)
        if match:
            result[(path, match[1])] = int(match[2])
    return result

old_complexity, new_complexity = complexities(before_findings), complexities(after_findings)
complexity_changes = [{'path': path, 'function': function,
                       'before': old_complexity[path, function],
                       'after': new_complexity[path, function]}
                      for path, function in sorted(old_complexity.keys() & new_complexity.keys())
                      if old_complexity[path, function] != new_complexity[path, function]]
if any(change['after'] > change['before'] for change in complexity_changes):
    raise RuntimeError('cognitive complexity increased')
lint = {'rechecked_units': list(selected), 'before': len(before_findings),
        'after': len(after_findings), 'added': sorted(canonical(after_findings) - canonical(before_findings)),
        'removed': sorted(canonical(before_findings) - canonical(after_findings)),
        'complexity_changes': complexity_changes, 'full_lint_green': False,
        'baseline_or_suppression_increase': False}
save('lint-comparison', lint)

permission = []
for app, identifier, original, length, primary in (
        ('system', 9862, 'PermissionSet Buffer', 30, 'Field_No::Scope, Field_No::AppID, Field_No::RoleID'),
        ('base', 9009, 'Permission Set Buffer', 20, 'Field_No::Type, Field_No::RoleID')):
    stem = f'PermissionSetBuffer_{identifier}'
    base = source / 'apps' / app / 'system/security/access_control/table'
    header = (base / f'{stem}.h').read_text()
    definitions = (base / f'{stem}.def.cpp').read_text()
    body = (base / f'{stem}.cpp').read_text()
    checks = [f'TableId kId{{{identifier}}}' in header,
              f'kName{{"{original}"}}' in header, f'::agiru::Code<{length}> RoleID' in header,
              primary in header, f'#include "{stem}.h"' in body,
              f'#include "{stem}.h"' in definitions,
              f'RegisterTable<{stem}_Table>' in definitions,
              ('.tableType = TableType::Temporary' in definitions) == (app == 'system'),
              ('.fieldClass = ::agiru::FieldClass::FlowField' in definitions) == (app == 'system')]
    if not all(checks):
        raise RuntimeError('real declaration preservation failed: ' + stem)
    permission.append({'app': app, 'id': identifier, 'name': original, 'stem': stem,
                       'role_id_length': length, 'checks': len(checks),
                       'all_green': True, 'execution_proof': False})
save('real-table-declarations', permission)
xmlports = []
for app, identifier, direction, encoding, root_name, trigger in (
        ('base', 9991, 'Both', 'MSDos', 'Coverage', 'OnBeforeInsertRecordCodeCoverage'),
        ('test_runner', 130471, 'Export', 'UTF16', 'CodeCoverageResults', 'OnAfterGetRecordALCodeCoverage')):
    stem = f'CodeCoverageDetailed_{identifier}'
    base = source / 'apps' / app / 'system/test_tools/code_coverage/xmlport'
    header = (base / f'{stem}.h').read_text()
    definitions = (base / f'{stem}.def.cpp').read_text()
    body = (base / f'{stem}.cpp').read_text()
    checks = [f'XmlPortId kId{{{identifier}}}' in header,
              'kName{"Code Coverage Detailed"}' in header,
              f'class {stem}_XmlPort : public XmlPort<{stem}_XmlPort>' in header,
              f'#include "{stem}.h"' in body, f'#include "{stem}.h"' in definitions,
              f'RegisterXmlPort<{stem}_XmlPort>' in definitions,
              '.format = XmlPortFormat::VariableText' in definitions,
              f'.direction = XmlPortDirection::{direction}' in definitions,
              f'.encoding = ::agiru::TextEncoding::{encoding}' in definitions,
              f'.rootName = "{root_name}"' in definitions,
              f'void {stem}_XmlPort::{trigger}()' in body,
              'is on a table this build does not carry (board:0065)' in body]
    if not all(checks):
        raise RuntimeError('real XMLport identity/metadata preservation failed: ' + stem)
    xmlports.append({'app': app, 'id': identifier, 'name': 'Code Coverage Detailed',
                     'stem': stem, 'checks': len(checks), 'all_green': True,
                     'table_dependency_refused': 'Code Coverage', 'execution_proof': False,
                     'full_schema_execution_proved': False})
save('real-xmlport-declarations', xmlports)
print(json.dumps({'source_sha256': identity['source_sha256'], 'generated_files': len(new),
                  'added_files': len(generated['added']), 'changed_files': len(generated['changed']),
                  'ut': ut, 'local_failures': comparison, 'lint': lint}, indent=2))
