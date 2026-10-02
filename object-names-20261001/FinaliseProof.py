from pathlib import Path
import importlib.util
import json
import re

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
source = root / 'source'

def read(name):
    return json.loads((artifacts / f'{name}.json').read_text())

identity = json.loads((root / 'source-identity.json').read_text())
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
if verify.digest(source) != identity['source_sha256']:
    raise RuntimeError('current source image changed after collection')
if verify.digest(Path(identity['origin'])) != identity['base_source_sha256']:
    raise RuntimeError('predecessor source image changed')
expected_exits = {'test-clang': 2, 'test-gcc': 2, 'gcc': 0, 'transpile': 2,
                  'lint-main': 2, 'lint-names': 2, 'lint-table': 2, 'lint-body': 2,
                  'lint-gate': 0}
exits = []
for name, expected in expected_exits.items():
    receipt = read(name)
    if receipt['exit'] != expected:
        raise RuntimeError('unexpected command status: ' + name)
    exits.append(receipt)
for compiler in ('clang', 'gcc'):
    current = read(f'generated-current-{compiler}')
    previous = read(f'generated-old-generator-{compiler}')
    if (current['tests'] != 5 or current['failures'] or current['errors'] or
            current['skipped'] or not current['success']):
        raise RuntimeError('current control population is not green')
    if (previous['tests'] != 5 or len(previous['failures']) != 5 or
            previous['errors'] or previous['skipped'] or previous['success']):
        raise RuntimeError('archived generator sensitivity changed')
    executions = [command for command in current['commands']
                  if command['stdout'] and 'GeneratedTableIdentity: 14 check(s), 0 red' in command['stdout']]
    if len(executions) != 2 or any(command['exit'] for command in executions):
        raise RuntimeError('both generated AL operation contexts must execute')
    log = (artifacts / f'test-{compiler}.log').read_text()
    for required in ('GenNames: 39 check(s), 0 red', 'Ran 117 tests',
                     'test: 92 case(s), 26 red'):
        if required not in log:
            raise RuntimeError('missing local population: ' + required)
    exits.extend([{'action': f'controls-current-{compiler}', 'exit': 0},
                  {'action': f'controls-old-generator-{compiler}', 'exit': 1}])
for comparison in read('local-failure-comparison'):
    if (comparison['before_failed_cases'] != 26 or comparison['after_failed_cases'] != 26 or
            comparison['added_failures'] or comparison['removed_failures']):
        raise RuntimeError('local failure identities changed')
ut = read('ut-population')
if (ut['before_codeunits'] != 80 or ut['after_codeunits'] != 80 or
        ut['before_methods'] != 2310 or ut['after_methods'] != 2310 or
        ut['missing'] or ut['added'] or ut['execution_proof']):
    raise RuntimeError('independent UT population changed or execution was claimed')
generated = read('generated-image')
if (generated['before_files'] != 24350 or generated['after_files'] != 24356 or
        generated['missing'] or len(generated['added']) != 6 or len(generated['changed']) != 13 or
        generated['complete_translation'] or generated['declaration_coverage_proved']):
    raise RuntimeError('partial generated image differs from reviewed result')
for declaration in read('real-table-declarations'):
    if not declaration['all_green'] or declaration['checks'] != 9 or declaration['execution_proof']:
        raise RuntimeError('real table preservation proof differs')
lint = read('lint-comparison')
if (lint['before'] != 48 or lint['after'] != 48 or lint['added'] or lint['removed'] or
        lint['full_lint_green'] or lint['baseline_or_suppression_increase']):
    raise RuntimeError('analysis findings changed')
diagnostic = (artifacts / 'transpile.log').read_text()
if ('system/test_tools/code_coverage/xmlport/CodeCoverageDetailed.h' not in diagnostic or
        'base                     1492 table(s), 1692 codeunit(s), 2627 page(s)' not in diagnostic):
    raise RuntimeError('real translation did not reach the reviewed remaining collision')
(root / 'proof-exits.json').write_text(json.dumps(exits, indent=2) + '\n')
print(json.dumps({'source_sha256': identity['source_sha256'],
                  'finalised': True, 'production_integrated': False,
                  'full_ut_green': False, 'goal_complete': False}, indent=2))
