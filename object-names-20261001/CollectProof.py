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
paths = ['src/gen/Names.h', 'src/gen/Names.cpp', 'src/gen/CodeunitWriter.h',
         'src/gen/TableWriter.h', 'src/gen/TableWriter.cpp', 'src/gen/BodyWriter.cpp',
         'src/tc/Main.cpp', 'test/gate/GenNamesGate.cpp', 'test/toolchain.py']
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
             'complete_translation': False, 'declaration_coverage_proved': False,
             'inherited_image_is_not_successful_regeneration': True}
save('generated-image', generated)
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

old_worktree = Path('/home/cosmo/Git/agiru-worktrees/goal-20260928')
if (old_worktree / 'src/gen/Names.cpp').read_bytes() != (origin / 'src/gen/Names.cpp').read_bytes():
    raise RuntimeError('Names baseline source does not match predecessor')
before_units = units(old_worktree / 'build/lint/tidy.log')
before_units.update(units(root.parent / 'implicit-key-20261001/artifacts/table-writer-targeted.log'))
before_units.update(units(root.parent / 'text-result-20261001/artifacts/body-writer-targeted.log'))
before_units.update(units(origin.parent / 'artifacts/tc-main-targeted.log'))
selected = {'src/tc/Main.cpp': 'main', 'src/gen/Names.cpp': 'names',
            'src/gen/TableWriter.cpp': 'table', 'src/gen/BodyWriter.cpp': 'body',
            'test/gate/GenNamesGate.cpp': 'gate'}
before_findings = set().union(*(before_units[name] for name in selected))
after_findings = set().union(*(units(artifacts / f'lint-{tag}-targeted.log')[name]
                              for name, tag in selected.items()))
lint = {'rechecked_units': list(selected), 'before': len(before_findings),
        'after': len(after_findings), 'added': sorted(after_findings - before_findings),
        'removed': sorted(before_findings - after_findings), 'full_lint_green': False,
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
print(json.dumps({'source_sha256': identity['source_sha256'], 'generated_files': len(new),
                  'added_files': len(generated['added']), 'changed_files': len(generated['changed']),
                  'ut': ut, 'local_failures': comparison, 'lint': lint}, indent=2))
