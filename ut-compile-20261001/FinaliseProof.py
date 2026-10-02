from pathlib import Path
import importlib.util
import json
import re

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
identity = json.loads((root / 'source-identity.json').read_text())
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
if verify.digest(source) != identity['source_sha256']:
    raise RuntimeError('current source changed after collection')
if verify.digest(Path(identity['origin'])) != identity['base_source_sha256']:
    raise RuntimeError('predecessor source changed')

def read(name):
    return json.loads((artifacts / f'{name}.json').read_text())

for compiler in ('clang', 'gcc'):
    log = (artifacts / f'test-{compiler}.log').read_text()
    for required in ('PlatformField: 295 check(s), 0 red', 'GenTableBinding: 45 check(s), 0 red',
                     'Option: 19 check(s), 0 red', 'Ran 119 tests', 'test: 92 case(s), 26 red'):
        if required not in log:
            raise RuntimeError('missing local proof: ' + required)
    current = read(f'field-boundary-current-{compiler}')
    old = read(f'field-boundary-old-image-{compiler}')
    if (current['compile_exit'] != 0 or current['execution_exit'] != 0 or
            'FieldBoundary: 97 check(s), 0 red' not in current['execution_stdout']):
        raise RuntimeError('native boundary proof failed')
    if (old['compile_exit'] != 0 or old['execution_exit'] != 1 or
            'FieldBoundary: 97 check(s), 76 red' not in old['execution_stdout']):
        raise RuntimeError('whole old-image negative control changed')
    generated = read(f'generated-field-{compiler}')
    if any(generated[name] != 0 for name in ('transpile_exit', 'compile_exit', 'execution_exit')):
        raise RuntimeError('generated AL consumer failed')
    contracts = read(f'contracts-{compiler}')
    if (contracts['candidates'] != 14 or contracts['matching'] != 9 or
            next(row for row in contracts['results'] if row['table'] == 'Field')['exit'] != 0):
        raise RuntimeError('native contract population changed')
for row in read('local-failure-comparison'):
    if (row['before_failed_cases'] != 26 or row['after_failed_cases'] != 26 or
            row['added_failures'] or row['removed_failures']):
        raise RuntimeError('local failures changed')
ut = read('ut-population')
if (ut['before_codeunits'] != 80 or ut['after_codeunits'] != 80 or
        ut['before_methods'] != 2310 or ut['after_methods'] != 2310 or ut['missing'] or ut['added']):
    raise RuntimeError('independent UT population changed')
before = (artifacts / 'ut-before.log').read_text()
after = (artifacts / 'ut.log').read_text()
if ('native field declaration mismatch: Field' not in before or
        'native field declaration mismatch: Field' in after or
        "no matching function for call to 'Hyperlink'" not in after or
        'PrivacyNoticeRecord->Link' not in after):
    raise RuntimeError('UT compiler blocker differs from reviewed evidence')
if 'UT MILESTONE: 0 of 2310 over 80 codeunits, 80 incomplete' not in after:
    raise RuntimeError('unexecuted UT results were not retained')
image = read('generated-image')
if (image['before_files'] != 24359 or image['after_files'] != 24359 or
        len(image['changed']) != 188 or image['missing'] or image['added'] or
        not image['slice_byte_identical']):
    raise RuntimeError('generated image or slice population changed')
repeat = read('repeat-output')
if repeat['changed_bytes_or_mtime'] or repeat['added'] or repeat['missing']:
    raise RuntimeError('generation is not repeatable')
for name in ('option-contracts', 'real-field-page'):
    if not read(name)['success']:
        raise RuntimeError('negative controls failed: ' + name)
for row in read('lint-comparison')['units']:
    if row['added'] or row['removed'] or row['before_findings'] != row['after_findings']:
        raise RuntimeError('targeted analysis findings changed')
for name in ('tc', 'gcc', 'transpile', 'transpile-repeat'):
    if read(name)['exit'] != 0:
        raise RuntimeError('unexpected build status: ' + name)
for name in ('test-clang', 'test-gcc', 'ut', 'lint-field', 'lint-codeunit', 'lint-table'):
    if read(name)['exit'] != 2:
        raise RuntimeError('unexpected retained failure status: ' + name)
print(json.dumps({'source_sha256': identity['source_sha256'], 'all_receipts_reconciled': True,
                  'production_integrated': False, 'G1_proved': False}), flush=True)
