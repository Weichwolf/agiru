from pathlib import Path
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
def read(name):
    return json.loads((artifacts / (name + '.json')).read_text())
changes = read('changes')
if verify.digest(origin) != identity['base_source_sha256'] or verify.digest(source) != changes['source_sha256']:
    raise RuntimeError('source image changed')
def functions(text):
    return dict(re.findall(r'^void (\w+)\(\) \{(.*?)^\}', text, re.M | re.S))
def tokens(text):
    return re.findall(r'"(?:[^"\\]|\\.)*"|\w+|[^\s]', text)
retained = {}
for gate, count in [('NativeObject', 11), ('GenTableBinding', 8)]:
    old, new = [functions((owner / 'test/gate' / (gate + 'Gate.cpp')).read_text()) for owner in (origin, source)]
    if len(old) != count or any(name not in new or tokens(body) != tokens(new[name]) for name, body in old.items()):
        raise RuntimeError('existing gate changed: ' + gate)
    retained[gate] = count
before = (origin / 'test/toolchain.py').read_text()
after = (source / 'test/toolchain.py').read_text()
old_literal, new_literal = "'intrinsic target has ID 2000000225'", "'intrinsic target has ID 2000000196'"
if before.count(old_literal) != 1 or before.replace(old_literal, new_literal) != after:
    raise RuntimeError('toolchain population/assertions changed beyond source ID')
def failures(path):
    text = path.read_text()
    return set(re.findall(r'FAIL\s+an (?:unknown )?exception left (\S+)', text)) | set(
        re.findall(r'FAIL\s+[^\n]*/test/gate/([^/:]+)Gate\.cpp:', text))
local = []
for compiler, name in [('clang', 'test'), ('gcc', 'test-gcc')]:
    old_failures = failures(origin.parent / 'artifacts' / (name + '.log'))
    new_failures = failures(artifacts / (name + '.log'))
    if old_failures != new_failures or len(new_failures) != 26:
        raise RuntimeError('local failure identities changed')
    log = (artifacts / (name + '.log')).read_text()
    for expected in ('test: 92 case(s), 26 red', 'Ran 119 tests', '\nOK\n',
                     'NativeObject: 1022 check(s), 0 red', 'GenTableBinding: 125 check(s), 0 red'):
        if expected not in log:
            raise RuntimeError('missing local result: ' + expected)
    controls = read('controls-' + compiler)
    if len(controls) != 4 or not all(row['valid'] for row in controls) or (
            '125 check(s), 19 red' not in controls[0]['stdout'] or
            '1022 check(s), 67 red' not in controls[2]['stdout']):
        raise RuntimeError('negative controls differ')
    generated = read('generated-' + compiler)
    if any(generated[key] != 0 for key in ('transpile_exit', 'compile_exit', 'execution_exit')) or '3 check(s), 0 red' not in generated['execution_stdout']:
        raise RuntimeError('generated fixture differs')
    contracts = read('contracts-' + compiler)
    if contracts['candidates'] != 17 or contracts['matching'] != 15 or {
            row['table'] for row in contracts['results'] if row['exit'] != 0} != {'PageMetadata', 'TenantLicenseState'}:
        raise RuntimeError('native source contracts differ')
    local.append({'compiler': compiler, 'before_failed_cases': 26, 'after_failed_cases': 26,
                  'added': [], 'removed': []})
for name in ('al-oracle', 'real-consumer', 'generated-controls', 'toolchain-control'):
    if not read(name)['success']:
        raise RuntimeError('proof failed: ' + name)
diagnostic = read('next-diagnostic')
if not diagnostic['success'] or len(diagnostic['results']) != 4 or not diagnostic['consumer_unchanged']:
    raise RuntimeError('next compiler failure not reproduced independently')
for number in (321, 639, 879):
    if not (source / f'build/CMakeFiles/agiru_slice.dir/Unity/unity_stable/{number}/root_cxx.cxx.o').is_file():
        raise RuntimeError('formerly failing root did not compile: ' + str(number))
for name, count in [('codeunitwriter', 10), ('tablewriter', 34)]:
    lint = read('lint-' + name + '-comparison')
    if lint['added'] or lint['before_findings'] != count or lint['after_findings'] != count:
        raise RuntimeError('targeted findings changed')
repeat = read('repeat-output')
if repeat['before_files'] != 24359 or repeat['after_files'] != 24359 or any(
        repeat[key] for key in ('missing', 'added', 'changed_bytes_or_mtime')):
    raise RuntimeError('repeat generation differs')
for name, expected in [('native', 0), ('tc', 0), ('gcc', 0), ('test', 2), ('test-gcc', 2),
                       ('transpile', 0), ('transpile-repeat', 0), ('gap', 2), ('ut', 2)]:
    if read(name)['exit'] != expected:
        raise RuntimeError('unexpected make status: ' + name)
if '41 of 14471 generated bodies compile' not in (artifacts / 'gap.log').read_text():
    raise RuntimeError('body denominator/prefix differs')
def population(manifest):
    return {(entry['id'], entry['name'], method) for entry in manifest for method in entry['methods']}
old_manifest = json.loads((origin.parent / 'artifacts/ut-milestone.log.manifest.json').read_text())
new_manifest = read('ut-milestone.log.manifest')
if len(old_manifest) != 80 or len(new_manifest) != 80 or len(population(new_manifest)) != 2310 or population(old_manifest) != population(new_manifest):
    raise RuntimeError('source-counted UT population differs')
ut = read('ut')
if not ut['source_unchanged_during_build'] or ut['source_after'] != changes['source_sha256']:
    raise RuntimeError('UT source image changed during build')
results = [json.loads(line) for line in (artifacts / 'ut-milestone.log.results.jsonl').read_text().splitlines()]
if len(results) != 2310 or any(row['passed'] for row in results):
    raise RuntimeError('UT population/results differ')
receipt = {**changes, 'original_gate_functions_token_identical': retained,
           'native_checks': 1022, 'binder_checks': 125, 'predecessor_red': [67, 19],
           'generated_al_checks': 9, 'generated_cpp_checks': 3, 'source_contracts': [15, 17],
           'local_failure_comparison': local, 'toolchain_tests': 119, 'repeat_files': 24359,
           'body_prefix': [41, 14471], 'ut_codeunits': 80, 'ut_methods': 2310, 'ut_executed': 0,
           'ut_run': read('ut-milestone.log.run'), 'ut_source_stable': True,
           'formerly_failing_roots_compiled': [321, 639, 879], 'next_diagnostic': diagnostic,
           'production_integrated': False, 'sealed_seed_activation_proof': False, 'g1_achieved': False,
           'sql_or_saved_settings_workflow_proof': False, 'success': True}
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'source_sha256': receipt['source_sha256'], 'success': True,
                  'native_checks': 1022, 'binder_checks': 125, 'ut_methods': 2310, 'ut_executed': 0}), flush=True)
