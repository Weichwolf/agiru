import hashlib
import json
from pathlib import Path
import re
import subprocess

task = Path(__file__).resolve().parent
main = task.parent.parent
artifacts = task / 'artifacts'
old_log = task.parent / 'object-catalogue-integration-20261002/artifacts/main-tests.log'
new_log = artifacts / 'main-tests.log'
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
assert set(new) - set(old) == {'PageTableField'} and new['PageTableField'] == (312, 0)
assert 'test: 95 case(s), 26 red' in new_text
assert 'Ran 136 tests' in new_text and '\nOK\n' in new_text
assert not re.search(r'skipped|missing executable|unknown exception', new_text, re.I)
assert new_text.count('Connection:') == old_text.count('Connection:') == 26
elf = []
for name in ('libagiru_al.so', 'libagiru_net.so', 'libagiru_db.so', 'libagiru_gen.so', 'libagiru_rt.so', 'agirutc'):
    path = main / 'build' / name
    result = subprocess.run(['readelf', '-d', str(path)], capture_output=True, text=True, check=True)
    assert 'libstdc++' not in result.stdout and 'libgcc_s' not in result.stdout
    elf.append({'path': str(path), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                'needed': re.findall(r'\(NEEDED\).*?\[([^]]+)\]', result.stdout)})
receipt = {'production_promoted': True, 'curated_files': 10,
           'previous_case_population': 94, 'current_case_population': 95,
           'unchanged_failure_identities': sorted(new_failed), 'database_failures': 26,
           'existing_suite_counts_or_status_losses': [], 'new_native_checks': 312,
           'toolchain_tests': 136, 'skipped_tests': 0, 'elf': elf,
           'compiler_source_binding_implemented': False, 'live_provider_implemented': False,
           'full_generated_tree_or_G1_proved': False,
           'TableDef_ABI_note': 'All generated consumers must be rebuilt before AL execution; no mixed-image execution was used.'}
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key != 'elf'}))
