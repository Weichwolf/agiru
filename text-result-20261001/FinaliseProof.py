from pathlib import Path
import json
import re
import shutil

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
origin = Path('/home/cosmo/Git/agiru/build/native-catalogue-20261001/source')
files = (source / 'build/first-gap/files').read_text().splitlines()
actual = {str(path.resolve()) for path in (source / 'apps').rglob('*.cpp') if path.is_file()}
if set(files) != actual or len(files) != len(actual):
    raise RuntimeError('complete body inventory does not reconcile')
shutil.copy2(source / 'build/first-gap/files', artifacts / 'body-files-current')
gap = (artifacts / 'body-gap-current.log').read_text()
matched = re.search(r'gap: (\d+) of (\d+) generated bodies compile, then (.+)', gap)
if matched is None or int(matched[2]) != len(files):
    raise RuntimeError('missing or inconsistent first-gap result')
prefix = int(matched[1])

def failures(path):
    failed = set()
    for line in path.read_text().splitlines():
        if not line.startswith('FAIL  '):
            continue
        exception = re.search(r'an exception left (\w+)', line)
        if exception:
            failed.add(exception[1])
            continue
        filename = re.search(r'/test/gate/(\w+)Gate\.cpp:', line)
        if filename:
            failed.add(filename[1])
            continue
        raise RuntimeError('unrecognized failure: ' + line)
    return failed

tests = {}
for compiler, filename in (('clang', 'local-tests-clang-final.log'), ('gcc', 'local-tests-gcc.log')):
    old = failures(origin / 'build/native-catalogue-proof' / f'final-{compiler}-tests.log')
    new = failures(artifacts / filename)
    tests[compiler] = {'cases': 92, 'red': len(new), 'toolchain_tests': 107,
                       'added_failed_gates': sorted(new - old),
                       'removed_failed_gates': sorted(old - new), 'failed_gates': sorted(new)}
    text = (artifacts / filename).read_text()
    if 'Ran 107 tests' not in text or 'test: 92 case(s), 26 red' not in text:
        raise RuntimeError('complete local test results are missing')
    if 'Text: 43 check(s), 0 red' not in text:
        raise RuntimeError('final Text denominator is missing')
(artifacts / 'local-failure-comparison.json').write_text(json.dumps(tests, indent=2) + '\n')
controls = {}
for compiler in ('clang', 'gcc'):
    for mode in ('current', 'old-generator', 'old-runtime'):
        receipt = json.loads((artifacts / f'generated-{mode}-{compiler}.json').read_text())
        controls[f'{mode}-{compiler}'] = {'exit': 0 if receipt['success'] else 1,
                                        'commands': [command['exit'] for command in receipt['commands']],
                                        'errors': receipt['errors'], 'skips': receipt['skipped']}
    old = (artifacts / f'text-old-run-{compiler}.log').read_text()
    if 'Text: 43 check(s), 10 red' not in old:
        raise RuntimeError('negative runtime denominator changed')
exits = {'copy': 0, 'transpiler_build_clang': 0, 'gcc_build': 0,
         'transpile_clang': 0, 'local_clang_make': 2, 'local_gcc_make': 2,
         'tests': tests, 'text_checks': 43, 'old_text_red': 10,
         'generated_checks': 13, 'generated_controls': controls,
         'regenerated_body_commands': json.loads((artifacts / 'regenerated-body-commands.json').read_text()),
         'lint_commands': json.loads((artifacts / 'lint-commands.json').read_text()),
         'body_gap_make': 2, 'body_gap_script': 1,
         'bodies': {'total': len(files), 'passed_prefix': prefix, 'first_failed': 1,
                    'unmeasured': len(files) - prefix - 1,
                    'first_failure': str(Path(matched[3]).relative_to(source / 'apps'))},
         'compile_measurement_first_attempt': {'exit': 1, 'cause': '/usr/bin/time unavailable',
                                               'retained_log': 'artifacts/regenerated-body-controls.log'},
         'no_ut_execution': True, 'full_body_success': False,
         'production_integrated': False, 'frozen_integration_run': False}
(root / 'proof-exits.json').write_text(json.dumps(exits, indent=2) + '\n')
print(json.dumps({'tests': tests, 'bodies': exits['bodies'], 'generated_controls': controls}, indent=2))
