from pathlib import Path
import difflib
import importlib.util
import json
import re

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
identity = json.loads((root / 'source-identity.json').read_text())
origin = Path(identity['origin'])
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
if verify.digest(origin) != identity['base_source_sha256']:
    raise RuntimeError('predecessor source image changed')
paths = {'include/platform/Field.h', 'include/platform/ReflectionOptions.h',
         'include/platform/TableMetadata.h', 'src/gen/CodeunitWriter.cpp', 'src/rt/Storage.cpp',
         'test/gate/GenTableBindingGate.cpp', 'test/gate/NativeObjectGate.cpp'}
old_files, new_files = set(verify.files(origin)), set(verify.files(source))
changed = {str(path) for path in old_files & new_files
           if (origin / path).read_bytes() != (source / path).read_bytes()}
added = {str(path) for path in new_files - old_files}
if old_files - new_files or added != {'include/platform/ReflectionOptions.h'}:
    raise RuntimeError('unexpected added/missing source paths')
handwritten = {path for path in changed | added if not path.startswith('apps/')}
if handwritten != paths:
    raise RuntimeError('unexpected handwritten blast radius: ' + str(handwritten))
if (origin / 'test/slice').read_bytes() != (source / 'test/slice').read_bytes():
    raise RuntimeError('the compiling slice changed')
identity.update(source_sha256=verify.digest(source), base_digest_reverified_unchanged=True,
                production_integrated=False, prototype_paths=sorted(paths))
(root / 'source-identity.json').write_text(json.dumps(identity, indent=2) + '\n')
def read(name):
    return json.loads((artifacts / (name + '.json')).read_text())
def save(name, value):
    (artifacts / (name + '.json')).write_text(json.dumps(value, indent=2) + '\n')
def population(manifest):
    return {(entry['id'], entry['name'], method) for entry in manifest for method in entry['methods']}
before = json.loads((origin.parent / 'artifacts/ut-milestone.log.manifest.json').read_text())
after = read('ut-milestone.log.manifest')
old, new = population(before), population(after)
if len(before) != 80 or len(after) != 80 or len(old) != 2310 or old != new:
    raise RuntimeError('the full UT population changed')
save('ut-population', {'before_codeunits': len(before), 'after_codeunits': len(after),
                       'before_methods': len(old), 'after_methods': len(new),
                       'missing': sorted(old - new), 'added': sorted(new - old), 'execution_proof': False})
generated = sorted(path for path in changed if path.startswith('apps/'))
save('generated-image', {'before_files': sum(str(path).startswith('apps/') for path in old_files),
                         'after_files': sum(str(path).startswith('apps/') for path in new_files),
                         'missing': [], 'added': [], 'changed': generated,
                         'slice_byte_identical': True, 'full_al_compilation_proved': False})
for label, selection in [('prototype', sorted(paths)), ('generated', generated)]:
    patch = []
    for path in selection:
        previous = (origin / path).read_text() if (origin / path).exists() else ''
        patch.extend(difflib.unified_diff(previous.splitlines(keepends=True),
                                        (source / path).read_text().splitlines(keepends=True),
                                        fromfile='before/' + path, tofile='after/' + path))
    (artifacts / (label + '.patch')).write_text(''.join(patch))
def failures(log):
    text = log.read_text()
    return set(re.findall(r'FAIL\s+an (?:unknown )?exception left (\S+)', text)) | set(
        re.findall(r'FAIL\s+[^\n]*/test/gate/([^/:]+)Gate\.cpp:', text))
rows = []
for compiler in ('clang', 'gcc'):
    previous = failures(origin.parent / 'artifacts' / ('test-' + compiler + '.log'))
    current = failures(artifacts / ('test-' + compiler + '.log'))
    rows.append({'compiler': compiler, 'before_failed_cases': len(previous),
                 'after_failed_cases': len(current), 'added_failures': sorted(current - previous),
                 'removed_failures': sorted(previous - current)})
    if previous != current or len(current) != 26:
        raise RuntimeError('local failure identities changed')
    log = (artifacts / ('test-' + compiler + '.log')).read_text()
    if 'test: 92 case(s), 26 red' not in log or 'Ran 119 tests' not in log or '\nOK\n' not in log:
        raise RuntimeError('complete local/toolchain denominators differ')
    for name, suite, count, old_red in [('controls', 'NativeObject', 891, 181),
                                        ('binding-controls', 'GenTableBinding', 82, 5)]:
        controls = read(name + '-' + compiler)
        if len(controls) != 2:
            raise RuntimeError('control image missing')
        for row in controls:
            red = old_red if row['image'] == 'old' else 0
            expected = f'{suite}: {count} check(s), {red} red'
            if (row['compile_exit'] != 0 or row['execution_exit'] != (1 if red else 0) or
                    expected not in row['stdout']):
                raise RuntimeError('positive/negative control differs')
    catalogue = read('contracts-' + compiler)
    failed = {row['table'] for row in catalogue['results'] if row['exit'] != 0}
    if catalogue['candidates'] != 16 or catalogue['matching'] != 14 or failed != {
            'PageMetadata', 'TenantLicenseState'}:
        raise RuntimeError('native catalogue differs')
    fixture = read('generated-' + compiler)
    if (any(fixture[key] != 0 for key in ('transpile_exit', 'compile_exit', 'execution_exit')) or
            'GeneratedNativeMetadata: 1 check(s), 0 red' not in fixture['execution_stdout']):
        raise RuntimeError('the source-backed AL metadata fixture did not execute')
    projection = read('projection-' + compiler)
    if len(projection) != 2:
        raise RuntimeError('projection control missing')
    for row in projection:
        red = 0 if row['image'] == 'current' else 1
        if (row['compile_exit'] != 0 or row['execution_exit'] != red or
                f'TableMetadataProjection: 8 check(s), {red} red' not in row['stdout']):
            raise RuntimeError('projection control differs')
    header = read('header-proof-' + compiler)
    if header['standalone_checks'] != 4 or any(row['exit'] != 0 for row in header['samples']):
        raise RuntimeError('standalone/header cost proof differs')
save('local-failure-comparison', rows)
if (root / 'Metadata.Codeunit.al').read_text().count('then Error(') != 39:
    raise RuntimeError('the AL metadata fixture denominator changed')
if not read('real-uncouple')['success']:
    raise RuntimeError('actual Uncouple consumer or old-header controls differ')
if not read('text-guid-diagnostic')['success']:
    raise RuntimeError('the new Text/Guid compiler diagnostic is not reproducible')
repeat = read('repeat-output')
if (repeat['before_files'] != 24359 or repeat['after_files'] != 24359 or
        any(repeat[key] for key in ('missing', 'added', 'changed_bytes_or_mtime'))):
    raise RuntimeError('repeat changed generated bytes or mtimes')
for label in ('codeunit', 'storage'):
    lint = read('lint-' + label + '-comparison')
    if lint['added'] or lint['after_findings'] > lint['before_findings']:
        raise RuntimeError('targeted findings increased')
current_ut = (artifacts / 'ut.log').read_text()
if 'UT MILESTONE: 0 of 2310 over 80 codeunits, 80 incomplete' not in current_ut:
    raise RuntimeError('the incomplete UT denominator differs')
if 'mismatch: Table Metadata' in current_ut:
    raise RuntimeError('the Table Metadata compiler blocker did not clear')
if ('include/type/Guid.h:226:10: error:' not in current_ut or
        'ImportConsolidationFromAPI.cpp:293:90' not in current_ut):
    raise RuntimeError('the next compiler blocker differs')
if len(generated) != 4:
    raise RuntimeError('the generated blast radius differs')
exits = [read(name) for name in ('tc', 'gcc', 'test-clang', 'test-gcc', 'transpile',
                               'transpile-repeat', 'ut', 'gap', 'lint-codeunit', 'lint-storage')]
(root / 'proof-exits.json').write_text(json.dumps(exits, indent=2) + '\n')
print(json.dumps({'source_sha256': identity['source_sha256'], 'changed_source_paths': sorted(paths),
                  'changed_generated_files': len(generated), 'ut_methods': len(new),
                  'ut_codeunits': len(after), 'local_failures': rows,
                  'production_integrated': False, 'G1_proved': False}), flush=True)
