import ast
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
artifacts = task / 'artifacts'
old_log = task.parent / 'page-table-field-integration-20261002/artifacts/main-tests.log'
promoted = len(sys.argv) > 1 and sys.argv[1] == 'main'
new_log = artifacts / ('main-tests.log' if promoted else 'final-tests.log')
before_source = task / 'before-source'
measured_source = main if promoted else source
suite = re.compile(r'^([\w -]+): (\d+) check\(s\), (\d+) red$', re.M)

def results(path):
    text = path.read_text()
    measured = {match[1]: (int(match[2]), int(match[3])) for match in suite.finditer(text)}
    exceptions = set(re.findall(r'^FAIL  an exception left ([\w -]+)$', text, re.M))
    failed = exceptions | {name for name, (_, red) in measured.items() if red}
    return text, measured, exceptions, failed

old_text, old, old_exceptions, old_failed = results(old_log)
new_text, new, new_exceptions, new_failed = results(new_log)
assert old_failed == new_failed and len(new_failed) == 26
assert old_exceptions == new_exceptions
assert all(new[name] == result for name, result in old.items())
assert set(new) - set(old) == {'GenSourceBinding'} and new['GenSourceBinding'] == (36, 0)
assert 'test: 96 case(s), 26 red' in new_text
assert 'Ran 138 tests' in new_text and '\nOK\n' in new_text
assert not re.search(r'skipped|missing executable|unknown exception', new_text, re.I)
assert new_text.count('Connection:') == old_text.count('Connection:') == 26

def python_tests(tree):
    parsed = ast.parse((tree / 'test/toolchain.py').read_text())
    return {declaration.name + '.' + method.name: ast.dump(method, include_attributes=False)
        for declaration in parsed.body if isinstance(declaration, ast.ClassDef)
        for method in declaration.body if isinstance(method, ast.FunctionDef) and method.name.startswith('test_')}

before_tests = python_tests(before_source)
after_tests = python_tests(measured_source)
assert len(before_tests) == 136 and len(after_tests) == 138
assert all(after_tests[identity] == body for identity, body in before_tests.items())
elf = []
for name in ('libagiru_al.so', 'libagiru_net.so', 'libagiru_db.so', 'libagiru_gen.so', 'libagiru_rt.so', 'agirutc'):
    path = measured_source / 'build' / name
    result = subprocess.run(['readelf', '-d', str(path)], capture_output=True, text=True, check=True)
    assert 'libstdc++' not in result.stdout and 'libgcc_s' not in result.stdout
    elf.append({'path': str(path), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                'needed': re.findall(r'\(NEEDED\).*?\[([^]]+)\]', result.stdout)})
receipt = dict(previous_case_population=95, current_case_population=96,
    unchanged_failure_identities=sorted(new_failed), database_failures=26,
    existing_suite_counts_or_status_losses=[], new_binding_checks=36,
    previous_toolchain_tests=136, toolchain_tests=138,
    previous_python_test_bodies_unchanged=True,
    new_python_test_identities=sorted(after_tests.keys() - before_tests.keys()), skipped_tests=0,
    elf=elf, ordinary_source_binding_implemented=True,
    native_source_input_or_contract_consumption_implemented=False,
    live_provider_implemented=False, full_generated_tree_or_G1_proved=False,
    production_promoted=promoted)
(artifacts / ('main-proof.json' if promoted else 'final-proof.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key != 'elf'}))
