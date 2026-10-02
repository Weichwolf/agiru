from collections import Counter
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

def tokens(text):
    return re.findall(r'"(?:[^"\\]|\\.)*"|\w+|[^\s]', text)

changes = read('changes')
require(verify.digest(origin) == identity['base_source_sha256'] == changes['predecessor_sha256'] and
        verify.digest(source) == changes['source_sha256'], 'source image changed')
require(set(changes['handwritten_changed']) == {'include/platform/ODataEdmType.h',
        'src/gen/CodeunitWriter.cpp', 'test/gate/NativeObjectGate.cpp',
        'test/gate/GenTableBindingGate.cpp'} and len(changes['generated_changed']) == 8 and
        changes['slice_unchanged'] and changes['source_paths_unchanged'], 'unexpected blast radius')
for filename, old_count, new_count in [('NativeObjectGate.cpp', 13, 14),
                                      ('GenTableBindingGate.cpp', 10, 11)]:
    def functions(owner):
        return dict(re.findall(r'^void (\w+)\(\) \{(.*?)^\}',
                    (owner / 'test/gate' / filename).read_text(), re.M | re.S))
    before, after = functions(origin), functions(source)
    require(len(before) == old_count and len(after) == new_count and
            all(name in after and tokens(body) == tokens(after[name]) for name, body in before.items()),
            'original gate functions changed: ' + filename)
require((origin / 'test/toolchain.py').read_bytes() == (source / 'test/toolchain.py').read_bytes(),
        'toolchain assertions changed')
writer = Path('src/gen/CodeunitWriter.cpp')
before, after = (origin / writer).read_text(), (source / writer).read_text()
old_binding = 'add("OData Edm Type", "2000000203");'
new_binding = 'add("OData Edm Type", "2000000179");'
require(before.count(old_binding) == after.count(new_binding) == 1 and
        before.replace(old_binding, new_binding) == after, 'generator changed beyond source ID')
consumer = Path('apps/base/system/integration/page/ODataEDMDefinitionCard.cpp')
def procedures(owner):
    return dict(re.findall(r'^[^\n]* ODataEDMDefinitionCard_Page::(\w+)\([^\n]*\) \{(.*?)^\}',
                (owner / consumer).read_text(), re.M | re.S))
before, after = procedures(origin), procedures(source)
original_procedures = ('GetEDMXML', 'SetEDMXML', 'OnAfterGetRecord', 'OnValidateEDMDefinitionXML')
require(all(name in before and name in after and tokens(before[name]) == tokens(after[name])
            for name in original_procedures), 'original AL procedure lowering changed')
fixture = (root / 'OData.Codeunit.al').read_text()
require(fixture.count('then Error(') == 13 and 'exit(13);' in fixture, 'AL guard population changed')

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
            'NativeObject: 1328 check(s), 0 red', 'GenTableBinding: 141 check(s), 0 red')),
            'local gate result missing')
    controls = read('controls-' + compiler)
    require(len(controls) == 4 and all(row['valid'] and row['compile_exit'] == 0 for row in controls),
            'whole-image controls incomplete')
    require({(row['gate'], row['image']) for row in controls} ==
            {(gate, image) for gate in ('NativeObject', 'GenTableBinding')
                           for image in ('predecessor', 'current')}, 'control images missing')
    for row in controls:
        total, reds = (1328, 12) if row['gate'] == 'NativeObject' else (141, 14)
        expected = reds if row['image'] == 'predecessor' else 0
        require(f'{total} check(s), {expected} red' in row['stdout'] and
                row['execution_exit'] == (1 if expected else 0), 'control result differs')
    generated = read('generated-' + compiler)
    require(all(generated[key] == 0 for key in ('transpile_exit', 'compile_exit', 'execution_exit')) and
            '1 check(s), 0 red' in generated['execution_stdout'], 'generated fixture differs')
    contracts = read('contracts-' + compiler)
    require(contracts['candidates'] == 18 and contracts['matching'] == 17 and
            {row['table'] for row in contracts['results'] if row['exit'] != 0} == {'TenantLicenseState'},
            'native source contracts differ')
    local.append({'compiler': compiler, 'before_failed_cases': 26, 'after_failed_cases': 26,
                  'added': [], 'removed': []})
for name in ('al-oracle', 'real-consumer', 'generated-controls', 'actual-odata'):
    require(read(name)['success'], 'proof failed: ' + name)
actual = read('actual-odata')
require(actual['temporary_store_only'] and actual['fixture_explicitly_installs_temporary_state'] and
        len(actual['results']) == 2 and all(row['valid'] and
        '12 check(s), 0 red' in row['stdout'] for row in actual['results']), 'actual page proof differs')
lint = read('lint-codeunitwriter-comparison')
require(not lint['added'] and lint['before_findings'] == lint['after_findings'] == 10 and
        not lint['baseline_raised'], 'targeted findings grew')
repeat = read('repeat-output')
require(repeat['before_files'] == repeat['after_files'] == 24359 and
        not any(repeat[key] for key in ('missing', 'added', 'changed_bytes_or_mtime')), 'repeat differs')
for name, expected in [('native', 0), ('binder', 0), ('tc', 0), ('gcc', 0), ('test', 2),
                       ('test-gcc', 2), ('transpile', 0), ('transpile-repeat', 0), ('gap', 2)]:
    require(read(name)['exit'] == expected, 'unexpected make status: ' + name)
require('41 of 14471 generated bodies compile' in (artifacts / 'gap.log').read_text(),
        'body denominator/prefix differs')
def population(manifest):
    return {(entry['id'], entry['name'], method) for entry in manifest for method in entry['methods']}
old_manifest = json.loads((origin.parent / 'artifacts/ut-milestone.log.manifest.json').read_text())
manifest = read('ut-milestone.log.manifest')
require(len(old_manifest) == len(manifest) == 80 and len(population(manifest)) == 2310 and
        population(old_manifest) == population(manifest), 'source-counted UT identities differ')
receipt = {**changes, 'original_native_gate_functions_token_identical': 13,
           'original_binder_gate_functions_token_identical': 10, 'native_checks': 1328,
           'predecessor_native_red': 12, 'binder_checks': 141, 'predecessor_binder_red': 14,
           'original_al_procedure_bodies_token_identical': list(original_procedures),
           'generated_al_guards': 13, 'generated_cpp_checks': 1, 'actual_page_temporary_checks': 12,
           'source_contracts': [17, 18], 'local_failure_comparison': local,
           'toolchain_tests_unchanged': 119, 'repeat_files': 24359, 'body_prefix': [41, 14471],
           'ut_codeunits': 80, 'ut_methods': 2310, 'production_integrated': False,
           'sealed_seed_activation_proof': False, 'g1_achieved': False,
           'sql_permissions_or_client_workflow_proof': False}
if sys.argv[1:] == ['--preflight']:
    receipt['preflight_success'] = True
    (artifacts / 'preflight-proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print(json.dumps({'preflight_success': True, 'source_sha256': changes['source_sha256']}), flush=True)
    raise SystemExit(0)
require(not sys.argv[1:], 'expected no argument or --preflight')
ut = read('ut')
require(ut['exit'] == 2 and ut['source_unchanged_during_build'] and
        ut['source_after'] == changes['source_sha256'], 'UT source image or exit differs')
results = [json.loads(line) for line in (artifacts / 'ut-milestone.log.results.jsonl').read_text().splitlines()]
require(len(results) == 2310 and
        {(row['codeunit_id'], row['codeunit'], row['method']) for row in results} == population(manifest),
        'UT result identities differ')
require(not any(row['passed'] for row in results) and
        all(row['status'] == 'missing' for row in results), 'unexpected UT execution; investigate full results')
diagnostic = read('next-diagnostic')
require(diagnostic['success'] and diagnostic['consumer_unchanged'] and len(diagnostic['results']) == 4,
        'next diagnostic not reproduced independently')
require((source / 'build/CMakeFiles/agiru_slice.dir/Unity/unity_stable/739/root_cxx.cxx.o').is_file(),
        'formerly failing OData root did not compile')
receipt.update({'ut_executed': 0, 'ut_statuses': dict(Counter(row['status'] for row in results)),
                'ut_run': read('ut-milestone.log.run'), 'ut_source_stable': True,
                'formerly_failing_root_compiled': 739, 'next_diagnostic': diagnostic, 'success': True})
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'success': True, 'source_sha256': changes['source_sha256'],
                  'ut_methods': 2310, 'ut_executed': 0, 'g1_achieved': False}), flush=True)
