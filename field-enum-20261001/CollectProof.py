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
previous, current = set(verify.files(origin)), set(verify.files(source))
if previous != current:
    raise RuntimeError('source paths were added or removed')
changed = {str(path) for path in current if (origin / path).read_bytes() != (source / path).read_bytes()}
handwritten = {path for path in changed if not path.startswith('apps/')}
expected = {'src/gen/BodyWriter.cpp', 'src/gen/CodeunitWriter.cpp',
            'src/gen/CodeunitWriter.h', 'test/gate/GenTableBindingGate.cpp'}
if handwritten != expected:
    raise RuntimeError('unexpected handwritten blast radius: ' + str(handwritten))
generated = changed - handwritten
if len(generated) != 8 or any('/page/' not in path or not path.endswith('.cpp') for path in generated):
    raise RuntimeError('unexpected generated blast radius: ' + str(generated))
if (origin / 'test/slice').read_bytes() != (source / 'test/slice').read_bytes():
    raise RuntimeError('the compiling slice changed')
patch = []
for path in sorted(changed):
    patch.extend(difflib.unified_diff((origin / path).read_text().splitlines(keepends=True),
                                    (source / path).read_text().splitlines(keepends=True),
                                    fromfile='before/' + path, tofile='after/' + path))
(artifacts / 'prototype.patch').write_text(''.join(patch))
def functions(text):
    return dict(re.findall(r'^void (\w+)\(\) \{(.*?)^\}', text, re.M | re.S))
def tokens(text):
    return re.findall(r'"(?:[^"\\]|\\.)*"|\w+|[^\s]', text)
old_gate, new_gate = [functions((owner / 'test/gate/GenTableBindingGate.cpp').read_text())
                      for owner in (origin, source)]
if len(old_gate) != 6 or any(name not in new_gate or tokens(body) != tokens(new_gate[name])
                            for name, body in old_gate.items()):
    raise RuntimeError('an existing binding gate was changed')
def read(name):
    return json.loads((artifacts / (name + '.json')).read_text())
def failures(path):
    text = path.read_text()
    return set(re.findall(r'FAIL\s+an (?:unknown )?exception left (\S+)', text)) | set(
        re.findall(r'FAIL\s+[^\n]*/test/gate/([^/:]+)Gate\.cpp:', text))
local_rows = []
for compiler in ('clang', 'gcc'):
    name = 'test' if compiler == 'clang' else 'test-gcc'
    old_failures = failures(origin.parent / 'artifacts' / ('test-' + compiler + '.log'))
    new_failures = failures(artifacts / (name + '.log'))
    if old_failures != new_failures or len(new_failures) != 26:
        raise RuntimeError('local failure identities changed')
    log = (artifacts / (name + '.log')).read_text()
    for expected_text in ('test: 92 case(s), 26 red', 'Ran 119 tests', '\nOK\n',
                          'Text: 75 check(s), 0 red', 'NativeObject: 891 check(s), 0 red',
                          'GenTableBinding: 105 check(s), 0 red'):
        if expected_text not in log:
            raise RuntimeError('local/toolchain result differs: ' + expected_text)
    controls = read('gate-controls-' + compiler)
    if len(controls) != 2 or not all(row['valid'] for row in controls):
        raise RuntimeError('whole-image binding controls differ')
    if '105 check(s), 7 red' not in controls[0]['stdout']:
        raise RuntimeError('the predecessor no longer demonstrates the defect')
    fixture = read('generated-' + compiler)
    if any(fixture[key] != 0 for key in ('transpile_exit', 'compile_exit', 'execution_exit')) or '4 check(s), 0 red' not in fixture['execution_stdout']:
        raise RuntimeError('generated AL execution differs')
    local_rows.append({'compiler': compiler, 'before_failed_cases': len(old_failures),
                       'after_failed_cases': len(new_failures), 'added': [], 'removed': []})
if not read('al-oracle')['success']:
    raise RuntimeError('AL compiler controls differ')
if not all(row['valid'] for row in read('generated-controls')):
    raise RuntimeError('old generator controls differ')
real = read('real-consumers')
if not real['success'] or len(real['report']) != 4 or len(real['feature']) != 4:
    raise RuntimeError('actual report/feature consumer proofs differ')
diagnostic = read('next-diagnostic')
if not diagnostic['success'] or len(diagnostic['results']) != 4 or not diagnostic['consumer_unchanged']:
    raise RuntimeError('the next compiler defect was not independently reproduced')
if not (source / 'build/CMakeFiles/agiru_slice.dir/Unity/unity_stable/639/root_cxx.cxx.o').is_file():
    raise RuntimeError('the formerly failing report root did not compile')
for label, count in [('bodywriter', 25), ('codeunitwriter', 10)]:
    lint = read('lint-' + label + '-comparison')
    if lint['added'] or lint['before_findings'] != count or lint['after_findings'] != count:
        raise RuntimeError('targeted analysis findings changed')
repeat = read('repeat-output')
if repeat['before_files'] != 24359 or repeat['after_files'] != 24359 or any(
        repeat[key] for key in ('missing', 'added', 'changed_bytes_or_mtime')):
    raise RuntimeError('repeat generation is unstable')
for name, expected_exit in [('binding', 0), ('tc', 0), ('gcc', 0), ('test', 2),
                            ('test-gcc', 2), ('transpile', 0), ('transpile-repeat', 0),
                            ('gap', 2), ('lint-bodywriter', 2), ('lint-codeunitwriter', 2), ('ut', 2)]:
    if read(name)['exit'] != expected_exit:
        raise RuntimeError('unexpected make exit: ' + name)
if '41 of 14471 generated bodies compile' not in (artifacts / 'gap.log').read_text():
    raise RuntimeError('the body denominator or failing prefix changed')
def population(manifest):
    return {(entry['id'], entry['name'], method) for entry in manifest for method in entry['methods']}
old_manifest = json.loads((origin.parent / 'artifacts/ut-milestone.log.manifest.json').read_text())
new_manifest = read('ut-milestone.log.manifest')
old_population, new_population = population(old_manifest), population(new_manifest)
if len(old_manifest) != 80 or len(new_manifest) != 80 or len(new_population) != 2310 or old_population != new_population:
    raise RuntimeError('the full independent UT population differs')
run = read('ut-milestone.log.run')
ut = read('ut')
if not ut['source_unchanged_during_build'] or ut['source_after'] != verify.digest(source):
    raise RuntimeError('the UT compiler image was not stable')
results = [json.loads(line) for line in (artifacts / 'ut-milestone.log.results.jsonl').read_text().splitlines()]
if len(results) != 2310 or any(row['passed'] for row in results):
    raise RuntimeError('UT result population differs')
receipt = {'source_sha256': verify.digest(source), 'predecessor_sha256': identity['base_source_sha256'],
           'handwritten_changed': sorted(handwritten), 'generated_changed': sorted(generated),
           'original_gate_functions_token_identical': True, 'binding_checks': 105,
           'predecessor_red_checks': 7, 'generated_al_checks': 8, 'generated_cpp_checks': 4,
           'real_feature_checks_per_image_and_compiler': 4, 'report_controls': 4,
           'local_failure_comparison': local_rows, 'toolchain_tests': 119,
           'repeat_files': 24359, 'body_prefix': [41, 14471],
           'ut_codeunits': 80, 'ut_methods': 2310, 'ut_executed': 0,
           'ut_run': run, 'ut_source_stable': True, 'report_root_639_compiled': True,
           'next_diagnostic': diagnostic, 'production_integrated': False,
           'sealed_seed_activation_proof': False, 'g1_achieved': False}
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'source_sha256': receipt['source_sha256'], 'success': True,
                  'binding_checks': 105, 'ut_methods': 2310, 'ut_executed': 0}), flush=True)
