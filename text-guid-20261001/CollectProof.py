from pathlib import Path
import difflib
import importlib.util
import json
import re
import sys

root = Path(__file__).resolve().parent
pending_ut = sys.argv[1:] == ['--pending-ut']
if sys.argv[1:] and not pending_ut:
    raise SystemExit('expected no arguments or --pending-ut')
source = root / 'source'
artifacts = root / 'artifacts'
identity = json.loads((root / 'source-identity.json').read_text())
origin = Path(identity['origin'])
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
if verify.digest(origin) != identity['base_source_sha256']:
    raise RuntimeError('predecessor source image changed')
previous, current = set(verify.files(origin)), set(verify.files(source))
if previous != current:
    raise RuntimeError('source paths were added or removed')
changed = {str(path) for path in current if (origin / path).read_bytes() != (source / path).read_bytes()}
expected = {'include/type/Guid.h', 'test/gate/TextGate.cpp'}
if changed != expected:
    raise RuntimeError('unexpected blast radius: ' + str(changed))
def read(name):
    return json.loads((artifacts / (name + '.json')).read_text())
def save(name, value):
    (artifacts / (name + '.json')).write_text(json.dumps(value, indent=2) + '\n')
patch = []
for path in sorted(changed):
    patch.extend(difflib.unified_diff((origin / path).read_text().splitlines(keepends=True),
                                    (source / path).read_text().splitlines(keepends=True),
                                    fromfile='before/' + path, tofile='after/' + path))
(artifacts / 'prototype.patch').write_text(''.join(patch))
old_gate, new_gate = [(owner / 'test/gate/TextGate.cpp').read_text() for owner in (origin, source)]
def functions(text):
    return dict(re.findall(r'^void (\w+)\(\) \{(.*?)^\}', text, re.M | re.S))
def tokens(text):
    return re.findall(r'"(?:[^"\\]|\\.)*"|\w+|[^\s]', text)
for name, body in functions(old_gate).items():
    if name not in functions(new_gate) or tokens(body) != tokens(functions(new_gate)[name]):
        raise RuntimeError('an existing Text gate was changed: ' + name)
claims = r'\bCHECK_(?:TRUE|TEXT|SILENT)\(\s*"([^"]+)"'
old_claims, new_claims = re.findall(claims, old_gate), re.findall(claims, new_gate)
if len(old_claims) != 43 or len(new_claims) != 75 or not set(old_claims) <= set(new_claims):
    raise RuntimeError('the Text denominator shrank')
mutant = artifacts / 'generated-mutant-include'
headers = {path.relative_to(source / 'include') for path in (source / 'include').rglob('*') if path.is_file()}
if headers != {path.relative_to(mutant) for path in mutant.rglob('*') if path.is_file()}:
    raise RuntimeError('counterfactual is not a complete header image')
if {str(path) for path in headers if (source / 'include' / path).read_bytes() != (mutant / path).read_bytes()} != {'type/Guid.h'}:
    raise RuntimeError('counterfactual changed another header')
def population(manifest):
    return {(entry['id'], entry['name'], method) for entry in manifest for method in entry['methods']}
old_manifest = json.loads((origin.parent / 'artifacts/ut-milestone.log.manifest.json').read_text())
new_manifest = read('ut-milestone.log.manifest')
old_population, new_population = population(old_manifest), population(new_manifest)
if len(old_manifest) != 80 or len(new_manifest) != 80 or len(new_population) != 2310 or old_population != new_population:
    raise RuntimeError('the full independent UT population differs')
save('ut-population', {'before_codeunits': len(old_manifest), 'after_codeunits': len(new_manifest),
                       'before_methods': len(old_population), 'after_methods': len(new_population),
                       'missing': [], 'added': [], 'execution_proof': False})
def failures(path):
    text = path.read_text()
    return set(re.findall(r'FAIL\s+an (?:unknown )?exception left (\S+)', text)) | set(
        re.findall(r'FAIL\s+[^\n]*/test/gate/([^/:]+)Gate\.cpp:', text))
local_rows = []
for compiler in ('clang', 'gcc'):
    old_failures = failures(origin.parent / 'artifacts' / ('test-' + compiler + '.log'))
    new_failures = failures(artifacts / ('test-' + compiler + '.log'))
    log = (artifacts / ('test-' + compiler + '.log')).read_text()
    if old_failures != new_failures or len(new_failures) != 26:
        raise RuntimeError('local failure identities changed')
    for expected_text in ('test: 92 case(s), 26 red', 'Ran 119 tests', '\nOK\n',
                          'Text: 75 check(s), 0 red', 'Guid: 18 check(s), 0 red',
                          'NativeObject: 891 check(s), 0 red', 'GenTableBinding: 82 check(s), 0 red'):
        if expected_text not in log:
            raise RuntimeError('local/toolchain result differs: ' + expected_text)
    local_rows.append({'compiler': compiler, 'before_failed_cases': len(old_failures),
                       'after_failed_cases': len(new_failures), 'added': [], 'removed': []})
    controls = read('gate-controls-' + compiler)
    if len(controls) != 3 or not all(row['valid'] for row in controls):
        raise RuntimeError('runtime negative controls differ')
    fixture = read('generated-' + compiler)
    if any(fixture[key] != 0 for key in ('transpile_exit', 'compile_exit', 'execution_exit')) or 'GeneratedTextGuidJoin: 2 check(s), 0 red' not in fixture['execution_stdout']:
        raise RuntimeError('generated AL execution differs')
    catalogue = read('contracts-' + compiler)
    if catalogue['candidates'] != 16 or catalogue['matching'] != 14 or {
            row['table'] for row in catalogue['results'] if row['exit'] != 0} != {'PageMetadata', 'TenantLicenseState'}:
        raise RuntimeError('native contracts changed')
save('local-failure-comparison', local_rows)
for name, count in [('syntax-controls', 8), ('generated-controls', 4), ('option-diagnostic', 4)]:
    receipt = read(name)
    if not receipt['success'] or len(receipt['results']) != count or not all(row['valid'] for row in receipt['results']):
        raise RuntimeError('compiler controls differ: ' + name)
    if any('redefinition' in row['diagnostics'] for row in receipt['results']):
        raise RuntimeError('a control mixed incompatible headers')
oracle = read('al-oracle')
if not oracle['success'] or len(oracle['results']) != 6:
    raise RuntimeError('AL compiler oracle differs')
if not read('secret-assignment-diagnostic')['success']:
    raise RuntimeError('the literal/variable distinction is not reproducible')
if (root / 'Join.Codeunit.al').read_text().count('then Error(') != 16:
    raise RuntimeError('AL execution denominator differs')
repeat = read('repeat-output')
if repeat['before_files'] != 24359 or repeat['after_files'] != 24359 or any(repeat[key] for key in ('missing', 'added', 'changed_bytes_or_mtime')):
    raise RuntimeError('repeat changed generated bytes or mtimes')
lint = read('lint-guid-comparison')
if lint['added'] or lint['before_findings'] != 28 or lint['after_findings'] != 28:
    raise RuntimeError('targeted lint findings changed')
if any(read(name)['exit'] != 0 for name in ('gcc', 'transpile', 'transpile-repeat')):
    raise RuntimeError('a required build/generation failed')
ut_log = (artifacts / 'ut.log').read_text()
if pending_ut and ('UT MILESTONE:' in ut_log or (artifacts / 'ut.json').exists()):
    raise RuntimeError('UT became terminal; collect without --pending-ut')
if not pending_ut and 'UT MILESTONE: 0 of 2310 over 80 codeunits, 80 incomplete' not in ut_log:
    raise RuntimeError('UT is still running or its incomplete denominator differs')
if 'return left + right.ToText();' in ut_log:
    raise RuntimeError('the Guid concatenation blocker did not clear')
errors = re.findall(r'^(.+?):(\d+):(\d+): (?:fatal )?error: (.+)$', ut_log, re.M)
former_root = source / 'build/CMakeFiles/agiru_slice.dir/Unity/unity_stable/321/root_cxx.cxx.o'
if not former_root.is_file():
    raise RuntimeError('the formerly failing root did not compile')
save('ut-build-blockers', {'errors': [{'path': path.removeprefix(str(source) + '/'),
                                     'line': int(line), 'column': int(column), 'message': message}
                                    for path, line, column, message in errors],
                           'executed': 0, 'incomplete_codeunits': 80,
                           'build_status': 'running' if pending_ut else 'terminal',
                           'former_failing_root_compiled': True, 'G1_proved': False})
save('generated-image', {'before_files': 24359, 'after_files': 24359, 'changed': [],
                         'added': [], 'missing': [], 'slice_byte_identical': True})
identity.update(source_sha256=verify.digest(source), base_digest_reverified_unchanged=True,
                prototype_paths=sorted(changed), production_integrated=False,
                text_checks_before=43, text_checks_after=75, old_checks_token_identical=True,
                ut_build_status='running' if pending_ut else 'terminal')
(root / 'source-identity.json').write_text(json.dumps(identity, indent=2) + '\n')
exits = [read(name) for name in ('text-gate', 'gcc', 'test-clang', 'test-gcc',
                               'transpile', 'transpile-repeat', 'gap', 'lint-guid')]
if not pending_ut:
    exits.append(read('ut'))
(root / 'proof-exits.json').write_text(json.dumps(exits, indent=2) + '\n')
print(json.dumps({'source_sha256': identity['source_sha256'], 'changed_source_paths': sorted(changed),
                  'ut_methods': len(new_population), 'ut_codeunits': len(new_manifest),
                  'local_failures': local_rows, 'compiler_errors': len(errors),
                  'ut_build_status': 'running' if pending_ut else 'terminal',
                  'production_integrated': False, 'G1_proved': False}), flush=True)
