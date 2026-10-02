from pathlib import Path
import importlib.util
import json

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
actions = {name: json.loads((artifacts / f'{name}.json').read_text())
           for name in ('gcc', 'transpile-final', 'test-clang-verified', 'test-gcc')}
for name, expected in (('gcc', 0), ('transpile-final', 0),
                       ('test-clang-verified', 2), ('test-gcc', 2)):
    if actions[name]['exit'] != expected:
        raise RuntimeError('unexpected result: ' + name)
controls = []
for compiler in ('clang', 'gcc'):
    for mode in ('current', 'old-generator'):
        control = json.loads((artifacts / f'generated-{mode}-{compiler}.json').read_text())
        if control['tests'] != 2 or control['errors'] or control['skipped']:
            raise RuntimeError('control population changed')
        if control['success'] != (mode == 'current'):
            raise RuntimeError('negative control lost its sensitivity')
        controls.append({'compiler': compiler, 'mode': mode,
                         'success': control['success'], 'tests': control['tests'],
                         'command_exits': [command['exit'] for command in control['commands']]})
generated = json.loads((artifacts / 'generated-image.json').read_text())
population = json.loads((artifacts / 'ut-population.json').read_text())
failures = json.loads((artifacts / 'local-failure-comparison.json').read_text())
lint = json.loads((artifacts / 'lint-comparison.json').read_text())
if generated['missing'] or generated['added'] or population['missing'] or population['added']:
    raise RuntimeError('population/image changed unexpectedly')
if any(row['added_failures'] or row['removed_failures'] for row in failures):
    raise RuntimeError('local failure identities changed')
if lint['new_failure_modes']:
    raise RuntimeError('new analysis findings remain')
receipt = {'source_sha256': identity['source_sha256'], 'actions': actions,
           'controls': controls, 'local_suite_green': False, 'full_lint_green': False,
           'full_ut_execution_proved': False, 'production_integrated': False,
           'committed': False, 'generated_paths': generated['after_files'],
           'generated_changed': len(generated['changed']),
           'ut_methods': population['after_methods'], 'unchanged_failure_cases': failures,
           'classification': 'progress: shared default-key semantics and truthful translation status'}
(root / 'proof-exits.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items()
                  if key not in ('actions', 'controls', 'unchanged_failure_cases')}, indent=2))
