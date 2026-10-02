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
pattern = re.compile(r'^([\w -]+): (\d+) check\(s\), (\d+) red$', re.M)

def results(path):
    text = path.read_text()
    measured = {match[1]: (int(match[2]), int(match[3])) for match in pattern.finditer(text)}
    exceptions = set(re.findall(r'^FAIL  an exception left ([\w -]+)$', text, re.M))
    return text, measured, exceptions, exceptions | {name for name, (_, red) in measured.items() if red}

old_text, old, old_exceptions, old_failed = results(task.parent / 'page-record-binding-integration-20261002/artifacts/main-tests.log')
new_text, new, new_exceptions, new_failed = results(artifacts / 'final-tests.log')
assert old_failed == new_failed and len(new_failed) == 26
assert old_exceptions == new_exceptions and set(old) == set(new)
assert all(new[name] == result for name, result in old.items() if name != 'GenSourceBinding')
assert old['GenSourceBinding'] == (89, 0) and new['GenSourceBinding'] == (159, 0)
assert 'test: 96 case(s), 26 red' in new_text and 'Ran 140 tests' in new_text and '\nOK\n' in new_text
assert not re.search(r'skipped|missing executable|unknown exception', new_text, re.I)
assert new_text.count('Connection:') == old_text.count('Connection:') == 26

def python_tests(tree):
    parsed = ast.parse((tree / 'test/toolchain.py').read_text())
    return {declaration.name + '.' + method.name: ast.dump(method, include_attributes=False)
        for declaration in parsed.body if isinstance(declaration, ast.ClassDef)
        for method in declaration.body if isinstance(method, ast.FunctionDef) and method.name.startswith('test_')}

old_tests = python_tests(main)
new_tests = python_tests(source)
assert len(old_tests) == len(new_tests) == 140
assert all(new_tests[identity] == body for identity, body in old_tests.items())
elf = []
for name in ('libagiru_al.so', 'libagiru_net.so', 'libagiru_db.so', 'libagiru_gen.so', 'libagiru_rt.so', 'agirutc'):
    path = source / 'build' / name
    result = subprocess.run(['readelf', '-d', str(path)], capture_output=True, text=True, check=True)
    assert 'libstdc++' not in result.stdout and 'libgcc_s' not in result.stdout
    elf.append({'path': str(path), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                'needed': re.findall(r'\(NEEDED\).*?\[([^]]+)\]', result.stdout)})
controls = json.loads((artifacts / 'final-controls.json').read_text())
mutant = json.loads((artifacts / 'normalized-field-mutant.json').read_text())
assert mutant['source_unchanged'] and mutant['exit'] == 1 and mutant['red'] == 1
generation = json.loads((artifacts / 'generation.json').read_text())
consumers = json.loads((artifacts / 'consumers.json').read_text())
census = json.loads((artifacts / 'census-comparison.json').read_text())
lint = json.loads((artifacts / 'targeted-lint-current.json').read_text())
assert not any(row['new_findings'] for row in lint['rows'])
assert lint['sources_unchanged'] and generation['compiler_inputs_unchanged'] and census['complete_inventory_bytes_equal']
assert all((main / path).read_bytes() == (source / path).read_bytes()
           for path in ('scripts/scope_inventory.py', 'scope.json', 'apps.json'))
assert verify.digest(task.parent / 'compiler-llvm-20261001/bc_source') == census['bc_input_sha256']
assert not consumers['lost'] and not consumers['new_diagnostics'] and not consumers['new_refusals']
assert controls['rows'][0]['gate_exit'] != 0 and controls['rows'][1]['gate_exit'] == 0
assert controls['rows'][0]['python_exit'] != 0 and controls['rows'][1]['python_exit'] == 0
receipt = dict(main_source_sha256=verify.digest(main), own_source_sha256=verify.digest(source),
    local_cases=96, unchanged_failure_identities=sorted(new_failed), database_failures=26,
    old_Cpp_suite_counts_and_statuses_unchanged_except_binding_checks=True,
    binding_checks_before=89, binding_checks=159,
    previous_toolchain_tests=140, toolchain_tests=140, old_Python_test_bodies_unchanged=True,
    new_Python_test_identities=sorted(new_tests.keys() - old_tests.keys()), skipped_tests=0, elf=elf,
    consumer_units=len(consumers['units']), consumer_before_green=consumers['before_green'],
    consumer_after_green=consumers['after_green'], generated_changed=generation['changed'],
    census_summary=census['summary'],
    census_reuse_basis='Same frozen AL input and byte-identical independent scanner, apps and scope policy; later edits affect only the generator and binding fixtures.',
    refused_option_before=consumers['refused_option_before'],
    refused_option_after=consumers['refused_option_after'],
    production_promoted=False, live_provider_implemented=False,
    complete_generated_tree_or_G1_proved=False)
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key != 'elf'}))
