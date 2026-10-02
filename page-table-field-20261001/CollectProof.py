import ast
import importlib.util
import json
from pathlib import Path
import re

root = Path(__file__).resolve().parent
source = root / 'source'
parent = root.parent / 'compiler-llvm-20261001'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)


def population(path):
    tree = ast.parse(path.read_text())
    return {(node.name, method.name) for node in tree.body if isinstance(node, ast.ClassDef)
            for method in node.body if isinstance(method, ast.FunctionDef)
            and method.name.startswith('test_')}


before = population(parent / 'source/test/toolchain.py')
after = population(source / 'test/toolchain.py')
assert before <= after and len(before) == 147 and len(after) == 149
tests = json.loads((artifacts / 'final-tests.json').read_text())
assert tests['exit'] == 2 and tests['source_unchanged']
assert verify.digest(source) == tests['source_after']
assert verify.digest(parent / 'source') == '1e0e824e772a1a1e611639434f23dc3476095a2fe8d0d68e408a2b6d13077970'
log = (artifacts / 'final-tests.log').read_text()
for text in ('Ran 149 tests', '\nOK\n', 'test: 94 case(s), 26 red',
             'PageTableField: 190 check(s), 0 red', 'GenTableBinding: 185 check(s), 0 red',
             'NativeObject: 1365 check(s), 0 red'):
    assert text in log, text
controls = json.loads((artifacts / 'source-controls.json').read_text())
assert [row['exit'] for row in controls['binder']] == [1, 0]
assert '185 check(s), 12 red' in controls['binder'][0]['stdout']
assert [row['exit'] for row in controls['contracts']] == [0, 1, 1, 1, 1, 1]
generated = json.loads((artifacts / 'generation.json').read_text())
assert generated['exit'] == 0 and not generated['missing_slice_sources']
assert generated['ut_codeunits'] == 80 and generated['ut_methods'] == 2310
raw = json.loads((artifacts / 'raw-manifest.json').read_text())
previous = json.loads((parent / 'artifacts/raw-manifest.json').read_text())
identities = lambda entries: {(row['id'], row['name'], method)
                             for row in entries for method in row['methods']}
assert identities(raw) == identities(previous)
pages = json.loads((artifacts / 'actual-pages.json').read_text())
assert len(pages['units']) == 6
failed = [row for row in pages['units'] if row['exit'] != 0]
assert len(failed) == 1 and failed[0]['source'].endswith('/AddPageFields.cpp')
assert failed[0]['diagnostics'].count('error:') == 8
assert 'NavDesignerProperty' in failed[0]['diagnostics']
current_lint = (source / 'build/lint/targeted.log').read_text()
old_lint = (parent / 'source/build/lint/targeted.log').read_text()


def diagnostics(text, path):
    return {re.sub(r':\d+:\d+:', ':LOC:', line.replace(str(path), '<source>'))
            for line in text.splitlines() if ': error:' in line}


current = diagnostics(current_lint, source)
old = diagnostics(old_lint, parent / 'source')
old_complexity = re.search(r"ProvisionInstalled' has cognitive complexity of (\d+)", old_lint)
new_complexity = re.search(r"ProvisionInstalled' has cognitive complexity of (\d+)", current_lint)
assert old_complexity and new_complexity
assert int(new_complexity[1]) < int(old_complexity[1])
assert not any("ProvisionSchema' has cognitive complexity" in line for line in current)
new_unexpected = current - old
assert all("ProvisionInstalled' has cognitive complexity" in line for line in new_unexpected), new_unexpected
receipt = {'source_sha256': tests['source_after'], 'parent_unchanged': True,
           'compiler_chain_production_integrated': False,
           'new_toolchain_methods': sorted(after - before), 'toolchain_green': 149,
           'local_cases': 94, 'local_red': 26, 'page_table_field_checks_green': 190,
           'binder_checks_green': 185, 'old_binder_checks_red': 12,
           'source_contract_negative_controls_red': 5,
           'ut_codeunits': 80, 'ut_methods': 2310, 'ut_identity_gains': 0, 'ut_identity_losses': 0,
           'standalone_consumer_units': 6, 'standalone_consumer_units_green': 5,
           'designer_errors': 8, 'live_provider_proved': False,
           'lint_green': False, 'lint_new_unexpected_findings': [],
           'provision_complexity_before': int(old_complexity[1]),
           'provision_complexity_after': int(new_complexity[1]), 'G1_proved': False}
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
