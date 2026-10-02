import ast
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
parent = root.parent / 'page-source-binding-20261001/source'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identity = verify.digest(source)
assert verify.digest(parent) == '6167d4c0faf2acc5b2a001e832d45c42beca0184461edba3c617272c35c17d2d'
final = json.loads((artifacts / 'dictionary-verified.json').read_text())
assert final['exit'] == 2 and final['source_unchanged']
assert identity == final['source_before'] == final['source_after']
log = (artifacts / 'dictionary-verified.log').read_text()
for text in ('Dictionary: 70 check(s), 0 red', 'DesignerConstants: 45 check(s), 0 red',
             'GenTableBinding: 213 check(s), 0 red', 'NativeObject: 1365 check(s), 0 red',
             'PageTableField: 190 check(s), 0 red', 'Ran 149 tests', '\nOK\n',
             'test: 96 case(s), 26 red'):
    assert text in log, text
assert 'skipped' not in log


def failures(text):
    escaped = set(re.findall(r'FAIL  an exception left (\w+)', text))
    red = {name for name, count in re.findall(r'^(\w+): \d+ check\(s\), (\d+) red$', text, re.M)
           if int(count) > 0}
    return sorted(escaped.union(red))


failed = failures(log)
assert failed == failures((parent.parent / 'artifacts/final-tests.log').read_text())
assert len(failed) == 26
assert 'Connection:' in log and 'test: missing executable' not in log


def methods(path):
    tree = ast.parse(path.read_text())
    return {(row.name, method.name) for row in tree.body if isinstance(row, ast.ClassDef)
            for method in row.body if isinstance(method, ast.FunctionDef)
            and method.name.startswith('test_')}


assert methods(source / 'test/toolchain.py') == methods(parent / 'test/toolchain.py')
assert len(methods(source / 'test/toolchain.py')) == 149
gates = {path.name for path in (source / 'test/gate').glob('*.cpp')}
old_gates = {path.name for path in (parent / 'test/gate').glob('*.cpp')}
assert old_gates.issubset(gates) and len(gates) + 3 == 96
assert gates - old_gates == {'DictionaryGate.cpp', 'DesignerConstantsGate.cpp'}
inputs = final['frozen_inputs']
compiler = root.parent / 'compiler-llvm-20261001'
assert verify.digest(compiler / 'bc_source') == inputs['bc_source_sha256']
assert verify.digest(compiler / 'system_symbols') == inputs['system_symbols_sha256']
generation = json.loads((artifacts / 'dictionary-verified/generation.json').read_text())
assert generation['exit'] == 0 and generation['source_after'] == identity
assert not generation['missing_slice_sources'] and not generation['deleted_outputs']
assert generation['generated_files'] == 24357 and generation['compiled_slice_sources'] == 14218
raw = json.loads((artifacts / 'dictionary-verified/raw-manifest.json').read_text())
assert raw == json.loads((parent.parent / 'artifacts/raw-manifest.json').read_text())
assert len(raw) == 80 and sum(len(row['methods']) for row in raw) == 2310
lint = json.loads((artifacts / 'lint-final-comparison.json').read_text())
assert len(lint) == 6 and all(not row['new_findings'] for row in lint)
consumers = json.loads((artifacts / 'actual-consumers-new.json').read_text())
assert consumers['source_sha256'] == identity and consumers['all_six_units_retained']
assert [row['exit'] for row in consumers['units']] == [0, 0, 0, 0, 1, 0]
errors = consumers['units'][4]['diagnostics']
assert errors.count('error:') == 1 and 'Keys' in errors
sweep = json.loads((artifacts / 'changed-bodies-final.json').read_text())
assert sweep['original_population'] == 16 and len(sweep['retained_original_comparisons']) == 16
assert sweep['source_sha256'] == identity and sweep['source_unchanged']
assert not sweep['new_compile_losses']
designer = json.loads((artifacts / 'designer-proof.json').read_text())
assert len(designer['complete_original_getters']) == len(designer['changed_value_controls']) == 20
assert designer['current_exit'] == 0
iterator = json.loads((artifacts / 'iterator-controls.json').read_text())
assert [row['exit'] for row in iterator] == [0, 1, 1]
old_receiver = json.loads((artifacts / 'receiver-old-replay.json').read_text())
new_receiver = json.loads((artifacts / 'receiver-new.json').read_text())
assert old_receiver['exit'] == 1 and '213 check(s), 3 red' in old_receiver['stdout']
assert new_receiver['exit'] == 0 and '213 check(s), 0 red' in new_receiver['stdout']
old_dictionary = json.loads((artifacts / 'dictionary-old.json').read_text())
new_dictionary = json.loads((artifacts / 'dictionary-new.json').read_text())
assert old_dictionary[0]['exit'] == 1 and '7 check(s), 6 red' in old_dictionary[0]['stdout']
assert new_dictionary[0]['exit'] == 0
elf = subprocess.run(['readelf', '-d', str(source / 'build/libagiru_net.so')],
                     capture_output=True, text=True, check=True).stdout
assert 'libstdc++' not in elf and all(name in elf for name in ('libc++.so', 'libc++abi.so', 'libunwind.so'))
outputs = {path.relative_to(source / 'apps').as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
           for path in (source / 'apps').rglob('*') if path.is_file()}
old_outputs = {path.relative_to(parent / 'apps').as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
               for path in (parent / 'apps').rglob('*') if path.is_file()}
assert outputs.keys() == old_outputs.keys()
changed = sorted(path for path in outputs if outputs[path] != old_outputs[path])
assert verify.digest(source) == identity
receipt = {'source_sha256': identity, 'parent_unchanged': True, 'frozen_inputs_rehashed': inputs,
           'dictionary_green': 70, 'designer_green': 45, 'designer_constants': 20,
           'designer_value_mutants_refused': 20, 'iterator_policy_mutants_refused': 2,
           'binder_green': 213, 'old_receiver_red': 3, 'old_dictionary_red': 6,
           'toolchain_green': 149, 'toolchain_missing_or_skipped': 0,
           'local_cases': 96, 'same_database_failed_cases': failed,
           'generated_files': len(outputs), 'changed_outputs_from_parent': changed,
           'deleted_outputs': [], 'active_slice_sources': 14218,
           'raw_UT_population': {'codeunits': 80, 'methods': 2310, 'identity_losses': 0},
           'original_comparison_bodies_retained': 16, 'new_compile_losses': [],
           'new_targeted_lint_findings': [], 'remaining_consumer_error': 'DictionaryWrapper.Keys',
           'production_integrated': False, 'SQL_workflow_or_full_tree_or_G1_proved': False}
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
