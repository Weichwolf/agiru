import ast
import json
from pathlib import Path
import re

task = Path(__file__).resolve().parent
root = task.parent.parent
source = task / 'source'
artifacts = task / 'artifacts'
pattern = re.compile(r'^([\w -]+): (\d+) check\(s\), (\d+) red$', re.M)

def gates(text):
    values = {row[1]: (int(row[2]), int(row[3])) for row in pattern.finditer(text)}
    exceptions = sorted(re.findall(r'^FAIL  an exception left ([\w -]+)$', text, re.M))
    return values, exceptions

old = (task.parent / 'source-binding-integration-20261002/artifacts/main-tests.log').read_text()
new = (artifacts / 'main-tests.log').read_text()
assert gates(old) == gates(new)
assert 'Ran 139 tests' in new and '\nOK\n' in new and 'test: 96 case(s), 26 red' in new
assert old.count('Connection:') == new.count('Connection:') == 26
assert not re.search(r'skipped|missing executable|unknown exception', new, re.I)

def tests(tree):
    parsed = ast.parse((tree / 'test/toolchain.py').read_text())
    return {item.name + '.' + method.name: ast.dump(method, include_attributes=False)
        for item in parsed.body if isinstance(item, ast.ClassDef)
        for method in item.body if isinstance(method, ast.FunctionDef) and method.name.startswith('test_')}

main_tests, own_tests = tests(root), tests(source)
assert len(main_tests) == 139 and len(own_tests) == 143
assert all(own_tests[name] == body for name, body in main_tests.items())
assert all(name.startswith('SystemSourceBindingGate.') for name in own_tests.keys() - main_tests.keys())
receipt = dict(promoted_files=['scripts/fetch_symbols.py', 'test/toolchain.py'],
    source_sha256=json.loads((artifacts / 'main-tests.json').read_text())['source_after'],
    toolchain_tests=139, local_cases=96, database_failures=26, old_Cpp_counts_and_statuses_unchanged=True,
    retained_old_Python_bodies=True, new_test='SymbolsPackageGate.test_verification_entrypoint_is_offline_and_read_only',
    offline_package_verification=True, normal_make_system_input_consumption=False,
    native_binding_promoted=False, G1_proved=False)
(artifacts / 'main-proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
