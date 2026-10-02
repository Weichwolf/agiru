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
paths = ['include/platform/RecordLink.h', 'src/gen/BodyWriter.cpp',
         'test/gate/GenTableBindingGate.cpp', 'test/gate/NativeObjectGate.cpp']
old_files, new_files = set(verify.files(origin)), set(verify.files(source))
changed = sorted(str(path) for path in old_files & new_files
                 if (origin / path).read_bytes() != (source / path).read_bytes())
unexpected = [path for path in changed if path not in paths and not path.startswith('apps/')]
added_source = {str(path) for path in new_files - old_files}
if added_source or old_files - new_files or unexpected:
    raise RuntimeError('unexpected source paths: ' + str(unexpected))
if (origin / 'test/slice').read_bytes() != (source / 'test/slice').read_bytes():
    raise RuntimeError('the compiling slice changed')
identity.update(source_sha256=verify.digest(source), base_digest_reverified_unchanged=True,
                production_integrated=False, prototype_paths=paths)
(root / 'source-identity.json').write_text(json.dumps(identity, indent=2) + '\n')
def save(name, value):
    (artifacts / (name + '.json')).write_text(json.dumps(value, indent=2) + '\n')
def read(name):
    return json.loads((artifacts / (name + '.json')).read_text())
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
save('generated-image', {'before_files': sum(path.startswith('apps/') for path in map(str, old_files)),
                         'after_files': sum(path.startswith('apps/') for path in map(str, new_files)),
                         'missing': [], 'added': [],
                         'changed': [path for path in changed if path.startswith('apps/')],
                         'slice_byte_identical': True, 'full_al_compilation_proved': False})
patch = []
for path in paths:
    patch.extend(difflib.unified_diff(((origin / path).read_text() if (origin / path).exists() else '').splitlines(keepends=True),
                                    (source / path).read_text().splitlines(keepends=True),
                                    fromfile='before/' + path, tofile='after/' + path))
(artifacts / 'prototype.patch').write_text(''.join(patch))
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
save('local-failure-comparison', rows)
for compiler in ('clang', 'gcc'):
    controls = read('controls-' + compiler)
    for row in controls:
        expected = 'NativeObject: 606 check(s), ' + ('25 red' if row['image'] == 'old' else '0 red')
        if row['compile_exit'] != 0 or expected not in row['stdout']:
            raise RuntimeError('native positive/negative control differs')
    catalogue = read('contracts-' + compiler)
    if catalogue['candidates'] != 16 or catalogue['matching'] != 13:
        raise RuntimeError('native catalogue differs')
    fixture = read('generated-' + compiler)
    if any(fixture[key] != 0 for key in ('transpile_exit', 'compile_exit', 'execution_exit')):
        raise RuntimeError('the original AL link fixture did not execute')
    if 'GeneratedNativeLink: 1 check(s), 0 red' not in fixture['execution_stdout']:
        raise RuntimeError('the original AL fixture wrapper differs')
    if set(row['table'] for row in catalogue['results'] if row['exit'] != 0) != {
            'PageMetadata', 'TableMetadata', 'TenantLicenseState'}:
        raise RuntimeError('native failure identities changed')
    for row in read('binding-controls-' + compiler):
        expected = 'GenTableBinding: 73 check(s), ' + ('16 red' if row['image'] == 'old' else '0 red')
        if row['compile_exit'] != 0 or expected not in row['stdout']:
            raise RuntimeError('generic binding positive/negative control differs')
    old_fixture = read('old-generated-' + compiler)
    if old_fixture['compile_exit'] == 0 or 'RecordID()' not in old_fixture['compile_stderr']:
        raise RuntimeError('the old emitter no longer exposes the collision')
    if old_fixture['execution_exit'] is not None:
        raise RuntimeError('an old-emitter failing image was executed')
    relation = read('relation-probe-' + compiler)
    if (relation['compile_exit'] != 0 or relation['execution_exit'] != 1 or
            'QualifiedRecordLinkRelation: 11 check(s), 4 red' not in relation['execution_stdout'] or
            relation['source_relation_removed']):
        raise RuntimeError('the retained qualified-relation diagnostic differs')
if (root / 'Link.Codeunit.al').read_text().count('then Error(') != 28:
    raise RuntimeError('the AL link fixture denominator changed')
if not read('real-crm-notes')['success']:
    raise RuntimeError('actual CRM Notes consumer or old-header controls differ')
repeat = read('repeat-output')
if (repeat['before_files'] != 24359 or repeat['after_files'] != 24359 or
        any(repeat[key] for key in ('missing', 'added', 'changed_bytes_or_mtime'))):
    raise RuntimeError('repeat changed generated bytes or mtimes')
for unit in ('body', 'storage'):
    lint = read('lint-' + unit + '-comparison')
    if lint['added'] or lint['after_findings'] > lint['before_findings']:
        raise RuntimeError('targeted findings increased')
if 'native key count mismatch: Record Link' not in (origin.parent / 'artifacts/ut.log').read_text():
    raise RuntimeError('the baseline compiler blocker differs')
current_ut = (artifacts / 'ut.log').read_text()
if ('native field count mismatch: Table Metadata' not in current_ut or
        'mismatch: Record Link' in current_ut or 'RecordLink.RecordID()' in current_ut):
    raise RuntimeError('the compiler blocker transition differs')
if 'UT MILESTONE: 0 of 2310 over 80 codeunits, 80 incomplete' not in current_ut:
    raise RuntimeError('the incomplete UT denominator differs')
if len([path for path in changed if path.startswith('apps/')]) != 4:
    raise RuntimeError('the generated blast radius differs')
save('generated-patch', {'files': [path for path in changed if path.startswith('apps/')]})
patch = []
for path in changed:
    if not path.startswith('apps/'):
        continue
    patch.extend(difflib.unified_diff((origin / path).read_text().splitlines(keepends=True),
                                    (source / path).read_text().splitlines(keepends=True),
                                    fromfile='before/' + path, tofile='after/' + path))
(artifacts / 'generated.patch').write_text(''.join(patch))
exits = [read(name) for name in ('binding-clang', 'tc', 'gcc', 'test-clang',
                               'test-gcc', 'transpile', 'transpile-repeat', 'ut', 'gap',
                               'lint-body', 'lint-storage')]
(root / 'proof-exits.json').write_text(json.dumps(exits, indent=2) + '\n')
print(json.dumps({'source_sha256': identity['source_sha256'], 'changed_source_paths': paths,
                  'changed_generated_files': len([path for path in changed if path.startswith('apps/')]),
                  'ut_methods': len(new), 'ut_codeunits': len(after), 'local_failures': rows,
                  'production_integrated': False, 'G1_proved': False}), flush=True)
