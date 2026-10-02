import ast
import difflib
import importlib.util
import json
from pathlib import Path

task = Path(__file__).resolve().parent
root = task.parent.parent
source = task / 'source'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
origin = json.loads((task / 'artifacts/origin.json').read_text())
assert verify.digest(root) == origin['origin_sha256'], 'main source changed since the reviewed origin'
patches = []

def patch(path, after):
    before = (root / path).read_text()
    assert before != after
    diff = ''.join(difflib.unified_diff(before.splitlines(True), after.splitlines(True), n=3))
    lines = ['@@' if line.startswith('@@') else line for line in diff.splitlines()[2:]]
    patches.append('*** Update File: ' + str(root / path) + '\n' + '\n'.join(lines) + '\n')

patch('scripts/fetch_symbols.py', (source / 'scripts/fetch_symbols.py').read_text())
own_tests = (source / 'test/toolchain.py').read_text()
tree = ast.parse(own_tests)
package_gate = next(item for item in tree.body if isinstance(item, ast.ClassDef) and item.name == 'SymbolsPackageGate')
test = next(item for item in package_gate.body if isinstance(item, ast.FunctionDef) and
            item.name == 'test_verification_entrypoint_is_offline_and_read_only')
method = ''.join(own_tests.splitlines(True)[test.lineno - 1:test.end_lineno])
before = (root / 'test/toolchain.py').read_text()
anchor = 'class SymbolsPackageGate(unittest.TestCase):\n'
assert before.count(anchor) == 1
after = before.replace(anchor, anchor + method + '\n')

def tests(text):
    return {item.name + '.' + method.name: ast.dump(method, include_attributes=False)
        for item in ast.parse(text).body if isinstance(item, ast.ClassDef)
        for method in item.body if isinstance(method, ast.FunctionDef) and method.name.startswith('test_')}

old = tests(before)
new = tests(after)
assert len(old) == 138 and len(new) == 139
assert all(new[name] == body for name, body in old.items())
assert new[package_gate.name + '.' + test.name] == tests(own_tests)[package_gate.name + '.' + test.name]
patch('test/toolchain.py', after)
print('*** Begin Patch\n' + ''.join(patches) + '*** End Patch')
