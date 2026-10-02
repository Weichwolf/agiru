import ast
import importlib.util
import json
from pathlib import Path

root = Path(__file__).resolve().parent
source = root / 'source'
parent = root.parent / 'page-table-field-20261001/source'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
final = json.loads((artifacts / 'final-tests.json').read_text())
assert final['exit'] == 2 and final['source_unchanged']
assert verify.digest(source) == final['source_before'] == final['source_after']
assert verify.digest(parent) == '13899bfd7676c8d0f821a4885b36c7ae3d7282887e058d806a4efe7c94ef19ec'
log = (artifacts / 'final-tests.log').read_text()
for text in ('GenTableBinding: 205 check(s), 0 red', 'NativeObject: 1365 check(s), 0 red',
             'PageTableField: 190 check(s), 0 red', 'Ran 149 tests', '\nOK\n',
             'test: 94 case(s), 26 red'):
    assert text in log, text
old = json.loads((artifacts / 'old-binding.json').read_text())
assert old['exit'] == 1 and '205 check(s), 18 red' in old['stdout']
generation = json.loads((artifacts / 'generation.json').read_text())
assert generation['exit'] == 0 and not generation['missing_slice_sources']
assert generation['generated_files'] == 24357 and generation['compiled_slice_sources'] == 14218
assert generation['changed_outputs'] == [
    'base/system/diagnostics/page/ChangeLogSetupFieldList.cpp',
    'base/system/security/user/page/UserCard.cpp']
assert generation['source_after'] == final['source_after']
expected = json.loads((artifacts / 'raw-manifest.json').read_text())
previous = json.loads((parent.parent / 'artifacts/raw-manifest.json').read_text())
assert expected == previous and len(expected) == 80
assert sum(len(row['methods']) for row in expected) == 2310


def methods(path):
    tree = ast.parse(path.read_text())
    return {(row.name, method.name) for row in tree.body if isinstance(row, ast.ClassDef)
            for method in row.body if isinstance(method, ast.FunctionDef)
            and method.name.startswith('test_')}


assert methods(source / 'test/toolchain.py') == methods(parent / 'test/toolchain.py')
assert len(methods(source / 'test/toolchain.py')) == 149
page_before = json.loads((artifacts / 'actual-page-before.json').read_text())
page_after = json.loads((artifacts / 'actual-page-after.json').read_text())
assert [row['exit'] for row in page_before['units']] == [1, 0]
assert [row['exit'] for row in page_after['units']] == [0, 0]
assert page_after['source_sha256'] == final['source_after']
user = json.loads((artifacts / 'actual-user.json').read_text())
assert [row['exit'] for row in user['units']] == [1, 0, 1, 0]
assert user['units'][0]['diagnostics'].count('error:') == 3
assert user['units'][2]['diagnostics'].count('error:') == 1
assert 'State_4' not in user['units'][2]['diagnostics']
assert 'IsWSKeyAllowed' in user['units'][2]['diagnostics']
assert user['source_sha256'] == final['source_after']
lint = json.loads((artifacts / 'lint-comparison.json').read_text())
assert lint['baseline_findings'] == lint['current_findings'] == 21 and not lint['new_findings']
designer = json.loads((artifacts / 'designer-contract.json').read_text())
assert len(designer['types']) == 2
assert sum(len(row['properties']) for row in designer['types']) == 20
receipt = {'source_sha256': final['source_after'], 'source_unchanged': True,
           'parent_unchanged': True, 'production_integrated': False,
           'binder_checks_green': 205, 'old_binder_red': 18,
           'native_object_checks_green': 1365, 'page_table_field_checks_green': 190,
           'toolchain_green': 149, 'local_cases': 94, 'local_red': 26,
           'actual_page_units': 4, 'actual_page_units_green_without_PCH': 3,
           'user_card_remaining_errors': ['NavTenantSettingsHelper.IsWSKeyAllowed'],
           'generated_files': 24357, 'changed_generated_files': generation['changed_outputs'],
           'deleted_generated_files': [], 'compiled_slice_sources': 14218,
           'ut_codeunits': 80, 'ut_methods': 2310, 'ut_identity_gains': 0, 'ut_identity_losses': 0,
           'lint_findings': 21, 'lint_new_findings': [], 'lint_green': False,
           'designer_original_static_integer_properties': 20,
           'page_execution_or_database_effects_proved': False, 'G1_proved': False}
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
