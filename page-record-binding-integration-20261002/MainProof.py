import ast
import importlib.util
import json
from pathlib import Path
import re

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
pattern = re.compile(r'^([\w -]+): (\d+) check\(s\), (\d+) red$', re.M)

def results(path):
    text = path.read_text()
    measured = {row[1]: (int(row[2]), int(row[3])) for row in pattern.finditer(text)}
    exceptions = sorted(re.findall(r'^FAIL  an exception left ([\w -]+)$', text, re.M))
    return text, measured, exceptions

own_text, own, own_exceptions = results(artifacts / 'final-tests.log')
main_text, current, main_exceptions = results(artifacts / 'main-tests.log')
assert own == current and own_exceptions == main_exceptions
assert 'Ran 140 tests' in main_text and '\nOK\n' in main_text
assert 'test: 96 case(s), 26 red' in main_text and current['GenSourceBinding'] == (89, 0)
assert own_text.count('Connection:') == main_text.count('Connection:') == 26
assert not re.search(r'skipped|missing executable|unknown exception', main_text, re.I)

def tests(tree):
    parsed = ast.parse((tree / 'test/toolchain.py').read_text())
    return {item.name + '.' + method.name: ast.dump(method, include_attributes=False)
        for item in parsed.body if isinstance(item, ast.ClassDef)
        for method in item.body if isinstance(method, ast.FunctionDef) and method.name.startswith('test_')}

assert tests(main) == tests(source) and len(tests(main)) == 140
generation = json.loads((artifacts / 'main-generation.json').read_text())
assert generation['source_matches_reviewed_image'] and generation['generated_matches_reviewed_image']
assert verify.digest(main) == verify.digest(source) == generation['source_after']
receipt = {'source_sha256': verify.digest(main), 'curated_files': 9,
    'toolchain_tests': 140, 'binding_checks': 89, 'local_cases': 96, 'database_failures': 26,
    'old_Cpp_and_Python_identities_counts_and_statuses_retained': True,
    'shared_page_record_binding_promoted': True,
    'generated_files': generation['actual_generated_files'], 'generated_matches_reviewed_image': True,
    'native_system_input_and_declaration_contracts_promoted': False,
    'live_provider_or_G1_proved': False}
(artifacts / 'main-proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
