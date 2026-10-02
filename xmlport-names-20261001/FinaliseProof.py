from pathlib import Path
import importlib.util
import json

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
exits = []
for name, expected in {'test-clang': 2, 'test-gcc': 2, 'gcc': 0, 'tc': 0, 'transpile-repeat': 0,
                       'lint-main': 2, 'lint-codeunit': 2, 'lint-table': 2,
                       'lint-body': 2, 'lint-page': 2}.items():
    receipt = read(name)
    if receipt['exit'] != expected:
        raise RuntimeError('unexpected command status: ' + name)
    exits.append(receipt)
for compiler in ('clang', 'gcc'):
    current = read(f'generated-current-{compiler}')
    previous = read(f'generated-old-generator-{compiler}')
    if (current['tests'] != 7 or current['failures'] or current['errors'] or
            current['skipped'] or not current['success']):
        raise RuntimeError('current controls are not all green')
    if (previous['tests'] != 7 or len(previous['failures']) != 6 or
            previous['errors'] or previous['skipped'] or previous['success']):
        raise RuntimeError('archived generator sensitivity changed')
    for fixture, count in (('Table', 14), ('XmlPort', 15)):
        message = f'Generated{fixture}Identity: {count} check(s), 0 red'
        executions = [command for command in current['commands']
                      if command['stdout'] and message in command['stdout']]
        if len(executions) != 2 or any(command['exit'] for command in executions):
            raise RuntimeError('both generated AL contexts must execute: ' + fixture)
    log = (artifacts / f'test-{compiler}.log').read_text()
    for message in ('GenNames: 39 check(s), 0 red', 'Ran 119 tests',
                    'test: 92 case(s), 26 red'):
        if message not in log:
            raise RuntimeError('local population changed: ' + message)
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
old_stems = ('base/system/test_tools/code_coverage/xmlport/CodeCoverageDetailed',
             'system/system/security/access_control/table/PermissionSetBuffer',
             'tests/core/table/TestTableC')
replaced = sorted(stem + suffix for stem in old_stems for suffix in ('.cpp', '.def.cpp', '.h'))
if (generated['before_files'] != 24356 or generated['after_files'] != 24359 or
        generated['missing'] != replaced or len(generated['added']) != 12 or
        len(generated['changed']) != 19 or generated['translation_exit'] != 0 or
        not generated['complete_translation'] or generated['full_al_compilation_proved'] or
        generated['declaration_coverage_proved']):
    raise RuntimeError('regenerated image differs from reviewed renames or overstates coverage')
slice_identity = read('slice-identity')
if (slice_identity['before'] != 14213 or slice_identity['after'] != 14219 or
        len(slice_identity['renamed']) != 6 or len(slice_identity['appended']) != 6 or
        slice_identity['current_missing_outputs'] or
        not slice_identity['unrelated_order_and_entries_unchanged'] or
        not slice_identity['original_ids_preserved'] or
        slice_identity['full_slice_compilation_proved']):
    raise RuntimeError('slice identity or monotonicity changed')
for declaration in read('real-table-declarations'):
    if not declaration['all_green'] or declaration['checks'] != 9 or declaration['execution_proof']:
        raise RuntimeError('real table preservation proof differs')
for declaration in read('real-xmlport-declarations'):
    if (not declaration['all_green'] or declaration['checks'] != 12 or
            declaration['table_dependency_refused'] != 'Code Coverage' or
            declaration['execution_proof'] or declaration['full_schema_execution_proved']):
        raise RuntimeError('real XMLport preservation or explicit refusal differs')
for compiler in ('clang', 'gcc'):
    declaration = read(f'real-tables-{compiler}')
    if (declaration['compile_exit'] or declaration['execution_exit'] or
            'RealTableIdentity: 20 check(s), 0 red' not in declaration['execution_stdout'] or
            declaration['database_execution_proof'] or declaration['flowfield_execution_proof']):
        raise RuntimeError('real temporary table execution differs')
repeat = read('repeat-output')
if (repeat['before_files'] != 24359 or repeat['after_files'] != 24359 or
        repeat['missing'] or repeat['added'] or repeat['changed_bytes_or_mtime']):
    raise RuntimeError('full generated repeat changed bytes, mtimes or paths')
lint = read('lint-comparison')
if (lint['after'] != lint['before'] - 1 or lint['added'] or len(lint['removed']) != 1 or
        lint['complexity_changes'] != [{'path': 'src/tc/Main.cpp', 'function': 'Scan',
                                       'before': 88, 'after': 86}] or
        lint['full_lint_green'] or lint['baseline_or_suppression_increase']):
    raise RuntimeError('analysis findings changed unexpectedly')
exits.append(read('transpile'))
(root / 'proof-exits.json').write_text(json.dumps(exits, indent=2) + '\n')
print(json.dumps({'source_sha256': identity['source_sha256'], 'finalised': True,
                  'production_integrated': False, 'full_ut_green': False,
                  'goal_complete': False}, indent=2))
