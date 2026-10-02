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
paths = ['include/platform/PrivacyNotice.h', 'include/platform/PrivacyNoticeApproval.h',
         'src/gen/CodeunitWriter.cpp', 'test/gate/NativeObjectGate.cpp',
         'test/gate/GenTableBindingGate.cpp']
old_files, new_files = set(verify.files(origin)), set(verify.files(source))
changed = sorted(str(path) for path in old_files & new_files
                 if (origin / path).read_bytes() != (source / path).read_bytes())
unexpected = [path for path in changed if path not in paths and not path.startswith('apps/')]
if old_files != new_files or unexpected:
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
    patch.extend(difflib.unified_diff((origin / path).read_text().splitlines(keepends=True),
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
        expected = 'NativeObject: 291 check(s), ' + ('34 red' if row['image'] == 'old' else '0 red')
        if row['compile_exit'] != 0 or expected not in row['stdout']:
            raise RuntimeError('native positive/negative control differs')
    catalogue = read('contracts-' + compiler)
    if catalogue['candidates'] != 16 or catalogue['matching'] != 11:
        raise RuntimeError('native catalogue differs')
    fixture = read('generated-' + compiler)
    if (fixture['compile_exit'] != 0 or fixture['execution_exit'] != 1 or
            'User-specific approval lookup' not in fixture['execution_stderr']):
        raise RuntimeError('the retained failing fixture differs')
    if read('guid-filter-' + compiler)['execution_exit'] != 1:
        raise RuntimeError('the GUID filter defect disappeared without a fix')
if not read('real-pages')['success']:
    raise RuntimeError('actual page declarations or old-header controls differ')
repeat = read('repeat-output')
if (repeat['before_files'] != 24359 or repeat['after_files'] != 24359 or
        any(repeat[key] for key in ('missing', 'added', 'changed_bytes_or_mtime'))):
    raise RuntimeError('repeat changed generated bytes or mtimes')
lint = read('lint-comparison')
if lint['added'] or lint['removed'] or lint['before_findings'] != lint['after_findings']:
    raise RuntimeError('targeted findings changed')
if 'PrivacyNoticeRecord->Link' not in (origin.parent / 'artifacts/ut.log').read_text():
    raise RuntimeError('the baseline compiler blocker differs')
current_ut = (artifacts / 'ut.log').read_text()
if 'native field count mismatch: All Profile' not in current_ut or 'Hyperlink(PrivacyNoticeRecord->Link)' in current_ut:
    raise RuntimeError('the compiler blocker transition differs')
exits = [read(name) for name in ('native-clang', 'binding-clang', 'tc', 'gcc', 'test-clang',
                               'test-gcc', 'transpile', 'transpile-repeat', 'ut', 'lint-codeunit')]
(root / 'proof-exits.json').write_text(json.dumps(exits, indent=2) + '\n')
print(json.dumps({'source_sha256': identity['source_sha256'], 'changed_source_paths': paths,
                  'changed_generated_files': len([path for path in changed if path.startswith('apps/')]),
                  'ut_methods': len(new), 'ut_codeunits': len(after), 'local_failures': rows,
                  'production_integrated': False, 'G1_proved': False}), flush=True)
