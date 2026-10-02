from pathlib import Path
import hashlib
import importlib.util
import json
import re
import sys

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

def require(condition, message):
    if not condition:
        raise RuntimeError(message)

changes = read('changes')
require(verify.digest(origin) == identity['base_source_sha256'] and
        verify.digest(source) == changes['source_sha256'], 'source image changed')
require(set(changes['handwritten_changed']) == {'include/platform/PageMetadata.h',
        'src/rt/Storage.cpp', 'test/gate/NativeObjectGate.cpp'} and not changes['generated_changed'] and
        changes['slice_unchanged'] and changes['source_paths_unchanged'], 'unexpected blast radius')

def functions(owner):
    return dict(re.findall(r'^void (\w+)\(\) \{(.*?)^\}',
                (owner / 'test/gate/NativeObjectGate.cpp').read_text(), re.M | re.S))

def tokens(text):
    return re.findall(r'"(?:[^"\\]|\\.)*"|\w+|[^\s]', text)

old, new = functions(origin), functions(source)
require(len(old) == 12 and len(new) == 13 and all(name in new and tokens(body) == tokens(new[name])
        for name, body in old.items()), 'original gate functions changed')
require((origin / 'test/toolchain.py').read_bytes() == (source / 'test/toolchain.py').read_bytes(),
        'toolchain population or assertions changed')
require((root / 'Metadata.Codeunit.al').read_text().count('then Error(') == 27 and
        'exit(27);' in (root / 'Metadata.Codeunit.al').read_text(), 'AL guard population changed')

def failures(path):
    log = path.read_text()
    return set(re.findall(r'FAIL\s+an (?:unknown )?exception left (\S+)', log)) | set(
        re.findall(r'FAIL\s+[^\n]*/test/gate/([^/:]+)Gate\.cpp:', log))

local = []
for compiler, name in [('clang', 'test'), ('gcc', 'test-gcc')]:
    before = failures(origin.parent / 'artifacts' / (name + '.log'))
    after = failures(artifacts / (name + '.log'))
    require(before == after and len(after) == 26, 'local failure identities changed')
    log = (artifacts / (name + '.log')).read_text()
    require(all(value in log for value in ('test: 92 case(s), 26 red', 'Ran 119 tests', '\nOK\n',
            'NativeObject: 1294 check(s), 0 red', 'GenTableBinding: 125 check(s), 0 red')),
            'missing local gate result')
    controls = read('controls-' + compiler)
    require(len(controls) == 2 and all(row['valid'] and row['compile_exit'] == 0 for row in controls) and
            '1294 check(s), 206 red' in controls[0]['stdout'] and
            '1294 check(s), 0 red' in controls[1]['stdout'], 'whole-image control differs')
    generated = read('generated-' + compiler)
    require(all(generated[key] == 0 for key in ('transpile_exit', 'compile_exit', 'execution_exit')) and
            '1 check(s), 0 red' in generated['execution_stdout'], 'generated fixture differs')
    projection = read('projection-' + compiler)
    require(len(projection) == 2 and all(row['compile_exit'] == 0 for row in projection) and
            projection[0]['execution_exit'] == 0 and '20 check(s), 0 red' in projection[0]['stdout'] and
            projection[1]['execution_exit'] == 1 and '20 check(s), 6 red' in projection[1]['stdout'],
            'projection or its negative control differs')
    contracts = read('contracts-' + compiler)
    require(contracts['candidates'] == 17 and contracts['matching'] == 16 and
            {row['table'] for row in contracts['results'] if row['exit'] != 0} == {'TenantLicenseState'},
            'native source contracts differ')
    local.append({'compiler': compiler, 'before_failed_cases': 26, 'after_failed_cases': 26,
                  'added': [], 'removed': []})

for name in ('al-oracle', 'real-consumer', 'generated-controls', 'actual-page-management'):
    require(read(name)['success'], 'proof failed: ' + name)
consumer = Path('apps/base/utilities/codeunit/PageManagement.cpp')
require((source / consumer).read_bytes() == (origin / consumer).read_bytes(), 'real consumer changed')
lint = read('lint-storage-comparison')
require(not lint['added'] and lint['before_findings'] == 57 and lint['after_findings'] == 56 and
        not lint['baseline_raised'], 'targeted findings grew')
repeat = read('repeat-output')
require(repeat['before_files'] == repeat['after_files'] == 24359 and
        not any(repeat[key] for key in ('missing', 'added', 'changed_bytes_or_mtime')), 'repeat differs')
for name, expected in [('native', 0), ('tc', 0), ('gcc', 0), ('test', 2), ('test-gcc', 2),
                       ('transpile', 0), ('transpile-repeat', 0), ('gap', 2)]:
    require(read(name)['exit'] == expected, 'unexpected make status: ' + name)
require('41 of 14471 generated bodies compile' in (artifacts / 'gap.log').read_text(),
        'body denominator/prefix differs')
receipt = {**changes, 'original_native_gate_functions_token_identical': 12, 'native_checks': 1294,
           'predecessor_native_red': 206, 'binder_checks_unchanged': 125, 'projection_checks': 20,
           'projection_mutant_red': 6, 'generated_al_guards': 27, 'generated_cpp_checks': 1,
           'actual_page_management_zero_table_guards': 2, 'source_contracts': [16, 17],
           'real_consumer_unchanged': True, 'real_consumer_sha256':
               hashlib.sha256((source / consumer).read_bytes()).hexdigest(),
           'local_failure_comparison': local, 'toolchain_tests_unchanged': 119,
           'repeat_files': 24359, 'body_prefix': [41, 14471], 'production_integrated': False,
           'sealed_seed_activation_proof': False, 'g1_achieved': False,
           'sql_provider_or_client_workflow_proof': False}
if sys.argv[1:] == ['--preflight']:
    print(json.dumps({'preflight_success': True, 'source_sha256': changes['source_sha256']}), flush=True)
    raise SystemExit(0)
require(not sys.argv[1:], 'expected no argument or --preflight')

def population(manifest):
    return {(entry['id'], entry['name'], method) for entry in manifest for method in entry['methods']}

old_manifest = json.loads((origin.parent / 'artifacts/ut-milestone.log.manifest.json').read_text())
manifest = read('ut-milestone.log.manifest')
require(len(old_manifest) == len(manifest) == 80 and len(population(manifest)) == 2310 and
        population(old_manifest) == population(manifest), 'source-counted UT identities differ')
ut = read('ut')
require(ut['exit'] == 2 and ut['source_unchanged_during_build'] and
        ut['source_after'] == changes['source_sha256'], 'UT source image or exit differs')
results = [json.loads(line) for line in (artifacts / 'ut-milestone.log.results.jsonl').read_text().splitlines()]
require(len(results) == 2310 and not any(row['passed'] for row in results), 'UT result population differs')
diagnostic = read('next-diagnostic')
require(diagnostic['success'] and diagnostic['consumer_unchanged'] and len(diagnostic['results']) == 4,
        'next diagnostic not reproduced independently')
require((source / 'build/CMakeFiles/agiru_slice.dir/Unity/unity_stable/792/root_cxx.cxx.o').is_file(),
        'formerly failing PageManagement root did not compile')
receipt.update({'ut_codeunits': 80, 'ut_methods': 2310, 'ut_executed': 0,
                'ut_run': read('ut-milestone.log.run'), 'ut_source_stable': True,
                'formerly_failing_root_compiled': 792, 'next_diagnostic': diagnostic, 'success': True})
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'success': True, 'source_sha256': changes['source_sha256'],
                  'ut_methods': 2310, 'ut_executed': 0, 'g1_achieved': False}), flush=True)
