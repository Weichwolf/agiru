import ast
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess

task = Path(__file__).resolve().parent
source = task / 'source'
main = task.parent.parent
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
suite = re.compile(r'^([\w -]+): (\d+) check\(s\), (\d+) red$', re.M)

def results(path):
    text = path.read_text()
    measured = {match[1]: (int(match[2]), int(match[3])) for match in suite.finditer(text)}
    exceptions = set(re.findall(r'^FAIL  an exception left ([\w -]+)$', text, re.M))
    return text, measured, exceptions, exceptions | {name for name, (_, red) in measured.items() if red}

old_text, old, old_exceptions, old_failed = results(task.parent / 'source-binding-integration-20261002/artifacts/main-tests.log')
new_text, new, new_exceptions, new_failed = results(artifacts / 'stable-tests.log')
assert old_failed == new_failed and len(new_failed) == 26
assert old_exceptions == new_exceptions
assert all(new[name] == result for name, result in old.items())
assert set(new) - set(old) == {'GenNativeBinding'} and new['GenNativeBinding'] == (40, 0)
assert 'test: 97 case(s), 26 red' in new_text and 'Ran 143 tests' in new_text and '\nOK\n' in new_text
assert not re.search(r'skipped|missing executable|unknown exception', new_text, re.I)
assert new_text.count('Connection:') == old_text.count('Connection:') == 26

def python_tests(tree):
    parsed = ast.parse((tree / 'test/toolchain.py').read_text())
    return {declaration.name + '.' + method.name: ast.dump(method, include_attributes=False)
        for declaration in parsed.body if isinstance(declaration, ast.ClassDef)
        for method in declaration.body if isinstance(method, ast.FunctionDef) and method.name.startswith('test_')}

old_tests = python_tests(main)
new_tests = python_tests(source)
assert len(old_tests) == 138 and len(new_tests) == 143
assert all(new_tests[identity] == body for identity, body in old_tests.items())
elf = []
for name in ('libagiru_al.so', 'libagiru_net.so', 'libagiru_db.so', 'libagiru_gen.so', 'libagiru_rt.so', 'agirutc'):
    path = source / 'build' / name
    result = subprocess.run(['readelf', '-d', str(path)], capture_output=True, text=True, check=True)
    assert 'libstdc++' not in result.stdout and 'libgcc_s' not in result.stdout
    elf.append({'path': str(path), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                'needed': re.findall(r'\(NEEDED\).*?\[([^]]+)\]', result.stdout)})
receipt = dict(main_source_sha256=verify.digest(main), own_source_sha256=verify.digest(source),
    previous_cases=96, cases=97, unchanged_failure_identities=sorted(new_failed), database_failures=26,
    old_Cpp_suite_counts_and_statuses_unchanged=True, new_native_binding_checks=40,
    previous_toolchain_tests=138, toolchain_tests=143, old_Python_test_bodies_unchanged=True,
    new_Python_test_identities=sorted(new_tests.keys() - old_tests.keys()), skipped_tests=0, elf=elf,
    production_promoted=False, original_tooling_fixture_ran=True,
    native_source_binding_and_make_consumption_implemented_in_own_image=True,
    direct_compiler_original_package_provenance_verified=False,
    live_provider_implemented=False, complete_generated_tree_or_G1_proved=False)
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key != 'elf'}))
