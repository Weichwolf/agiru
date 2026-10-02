import hashlib
import importlib.util
import json
from pathlib import Path

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
build = json.loads((artifacts / 'ut-resume.json').read_text())
assert build['exit'] == 2 and build['source_unchanged'] and build['frozen_inputs_unchanged']
assert verify.digest(source) == build['source_before'] == build['source_after']
run = json.loads((source / 'build/ut.log.run.json').read_text())
assert run['build_exit'] == 2 and run['status'] == 2
assert run['infrastructure_errors'] == ['build exited 2; the UT runner was not started']
assert run['source_revision'] == build['frozen_inputs']['bc_revision']
manifest = json.loads((source / 'build/ut.log.manifest.json').read_text())
previous = json.loads((root.parent / 'page-table-field-20261001/source/build/ut.log.manifest.json').read_text())
identities = lambda entries: {(entry['id'], entry['name'], method)
                              for entry in entries for method in entry['methods']}
expected = identities(manifest)
assert expected == identities(previous) and len(manifest) == 80 and len(expected) == 2310
results = [json.loads(line) for line in (source / 'build/ut.log.results.jsonl').read_text().splitlines()]
assert {(row['codeunit_id'], row['codeunit'], row['method']) for row in results} == expected
assert len(results) == len(expected)
assert all(row['status'] == 'missing' and row['passed'] is False for row in results)
log = (artifacts / 'ut-resume.log').read_text()
assert log.count('error:') == 3
assert '/619/root_cxx.cxx.o' in log
assert 'native field count mismatch: Tenant License State' in log
assert 'native field declaration mismatch: Tenant License State.State' in log
assert 'native field declaration mismatch: Tenant License State.User Security ID' in log
original = (artifacts / 'ut.log').read_text()
assert '/534/root_cxx.cxx.o' in original
assert 'ChangeLogSetupFieldList.cpp' not in log
path = Path('apps/system/system/environment/codeunit/TenantLicenseStateImpl.cpp')
parent_path = root.parent / 'page-table-field-20261001/source' / path
assert (source / path).read_bytes() == parent_path.read_bytes()
for suffix in ('manifest.json', 'results.jsonl', 'run.json'):
    (artifacts / ('ut-resume.' + suffix)).write_bytes((source / ('build/ut.log.' + suffix)).read_bytes())
receipt = {'source_sha256': build['source_after'], 'source_unchanged': True,
           'build_exit': 2, 'resumed_elapsed_seconds': build['elapsed_seconds'],
           'previous_handle_missing_without_terminal_receipt': True,
           'original_log_retained': True, 'runner_started': False,
           'codeunits': 80, 'methods': 2310, 'executed': 0, 'passed': 0,
           'missing': len(results), 'incomplete_codeunits': 80,
           'identity_gains': 0, 'identity_losses': 0, 'compiler_errors': 3,
           'first_compiler_error': 'TenantLicenseStateImpl.cpp:30: native field count mismatch',
           'failing_consumer_bytes_equal_parent': True,
           'failing_consumer_sha256': hashlib.sha256((source / path).read_bytes()).hexdigest(),
           'licensing_dependency_retirement_not_proved': True,
           'G1_proved': False}
(artifacts / 'ut-proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
