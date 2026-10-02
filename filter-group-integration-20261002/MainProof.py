import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
proof = json.loads((artifacts / 'proof.json').read_text())
assert verify.digest(main) == verify.digest(source) == proof['own_source_sha256']
tests = json.loads((artifacts / 'main-tests.json').read_text())
assert tests['source_unchanged'] and tests['source_after'] == proof['own_source_sha256']
pattern = re.compile(r'^([\w -]+): (\d+) check\(s\), (\d+) red$', re.M)

def results(path):
    text = path.read_text()
    measured = {match[1]: (int(match[2]), int(match[3])) for match in pattern.finditer(text)}
    exceptions = set(re.findall(r'^FAIL  an exception left ([\w -]+)$', text, re.M))
    return text, measured, exceptions

old_text, old, old_exceptions = results(artifacts / 'stable-tests.log')
new_text, new, new_exceptions = results(artifacts / 'main-tests.log')
assert old == new and old_exceptions == new_exceptions
assert 'test: 97 case(s), 26 red' in new_text and 'Ran 140 tests' in new_text and '\nOK\n' in new_text
assert new_text.count('Connection:') == old_text.count('Connection:') == 26
elf = []
for name in ('libagiru_al.so', 'libagiru_net.so', 'libagiru_db.so', 'libagiru_gen.so', 'libagiru_rt.so', 'agirutc'):
    path = main / 'build' / name
    dynamic = subprocess.check_output(['readelf', '-d', str(path)], text=True)
    assert 'libstdc++' not in dynamic and 'libgcc_s' not in dynamic
    elf.append({'name': name, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                'needed': re.findall(r'\(NEEDED\).*?\[([^]]+)\]', dynamic)})
receipt = {'source_sha256': proof['own_source_sha256'], 'own_and_main_complete_images_equal': True,
    'source_unchanged_during_main_tests': True, 'filter_group_checks': 137,
    'toolchain_tests': 140, 'local_cases': 97, 'unchanged_database_failure_identities': proof['unchanged_failure_identities'],
    'all_previous_test_counts_and_statuses_retained': True, 'elf': elf,
    'SQL_or_BC_setter_return_or_full_tree_or_G1_proved': False}
(artifacts / 'main-proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key != 'elf'}))
