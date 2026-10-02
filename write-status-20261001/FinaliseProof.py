from pathlib import Path
import importlib.util
import json
import re

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
identity = json.loads((root / 'source-identity.json').read_text())
source = root / 'source'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
if verify.digest(source) != identity['source_sha256']:
    raise RuntimeError('source image changed after collection')
if verify.digest(Path(identity['origin'])) != identity['base_source_sha256']:
    raise RuntimeError('predecessor image changed')

def read(name):
    return json.loads((artifacts / f'{name}.json').read_text())

actions = {name: read(name) for name in ('gcc', 'transpile', 'transpile-repeat',
                                       'test-clang', 'test-gcc', 'lint')}
for name, expected in (('gcc', 0), ('transpile', 0), ('transpile-repeat', 0),
                       ('test-clang', 2), ('test-gcc', 2), ('lint', 2)):
    if actions[name]['exit'] != expected:
        raise RuntimeError('unexpected result: ' + name)
controls = []
for compiler in ('clang', 'gcc'):
    for mode in ('current', 'old-generator'):
        control = read(f'generated-{mode}-{compiler}')
        if control['tests'] != 3 or control['errors'] or control['skipped']:
            raise RuntimeError('control population changed')
        if control['success'] != (mode == 'current'):
            raise RuntimeError('negative control lost sensitivity')
        if len(control['failures']) != (0 if mode == 'current' else 5):
            raise RuntimeError('negative-control failure population changed')
        controls.append({'compiler': compiler, 'mode': mode,
                         'success': control['success'], 'tests': control['tests'],
                         'failures': len(control['failures'])})
    log = (artifacts / f'test-{compiler}.log').read_text()
    if not re.search(r'Ran 112 tests.*?\nOK\n', log, re.S):
        raise RuntimeError('complete toolchain suite changed')
    if 'test: 92 case(s), 26 red' not in log:
        raise RuntimeError('complete local suite changed')
generated = read('generated-image')
population = read('ut-population')
failures = read('local-failure-comparison')
lint = read('lint-comparison')
repeat = read('repeat-output')
if generated['missing'] or generated['added'] or generated['changed']:
    raise RuntimeError('generated source image changed')
if population['missing'] or population['added'] or population['after_methods'] != 2310:
    raise RuntimeError('source population changed')
if any(row['added_failures'] or row['removed_failures'] or row['after_failed_cases'] != 26
       for row in failures):
    raise RuntimeError('local failure identities changed')
complexity = re.compile(r"function 'Scan' has cognitive complexity of ([0-9]+) \(threshold 25\)")
if lint['added'] or lint['removed']:
    if len(lint['added']) != 1 or len(lint['removed']) != 1:
        raise RuntimeError('new analysis finding')
    before = complexity.search(lint['removed'][0][1])
    after = complexity.search(lint['added'][0][1])
    if not before or not after or int(after[1]) >= int(before[1]):
        raise RuntimeError('analysis did not improve')
receipt = {'source_sha256': identity['source_sha256'], 'actions': actions,
           'controls': controls, 'local_suite_green': False, 'full_lint_green': False,
           'full_ut_execution_proved': False, 'production_integrated': False,
           'committed': False, 'generated_paths': generated['after_files'],
           'generated_changed': len(generated['changed']),
           'ut_methods': population['after_methods'], 'repeat_output': repeat,
           'classification': 'progress: shared checked output ownership; no cleanup after failure'}
(root / 'proof-exits.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key not in ('actions', 'controls')},
                 indent=2))
