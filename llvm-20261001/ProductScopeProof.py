import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
main = root.parent.parent
current = json.loads((artifacts / 'product-raw-ut.json').read_text())
previous = json.loads((main / 'build/chart-20261001/artifacts/ut-milestone.log.manifest.json').read_text())


def identities(entries):
    return {(item['id'], item['name'], method)
            for item in entries for method in item['methods']}


old_ids = identities(previous)
new_ids = identities(current)
assert old_ids == new_ids
assert len(current) == 80 and len(new_ids) == 2310
before = json.loads((artifacts / 'product-scope-old-stable.json').read_text())
after = json.loads((artifacts / 'product-scope-current.json').read_text())
suite = json.loads((artifacts / 'product-scope-tests.json').read_text())
assert all(row['source_unchanged'] for row in (before, after, suite))
assert before['exit'] == 2 and after['exit'] == 0 and suite['exit'] == 2
assert after['source_after'] == suite['source_before'] == suite['source_after']
assert '258 check(s), 6 red' in (artifacts / 'product-scope-old-stable.log').read_text()
assert '255 check(s), 0 red' in (artifacts / 'product-scope-current.log').read_text()
log = (artifacts / 'product-scope-tests.log').read_text()
assert 'Ran 105 tests' in log and '\nOK\n' in log
assert 'test: 89 case(s), 26 red' in log
assert 'provisioning still contains TenantLicenseState' in (
    artifacts / 'product-provisioning-old.log').read_text()
files = ('src/rt/Storage.cpp', 'src/gen/scope.json',
         'test/gate/GenScopeGate.cpp', 'test/toolchain.py')
hashes = {}
for name in files:
    code = (root / 'source' / name).read_bytes()
    assert code == (main / name).read_bytes(), name
    hashes[name] = hashlib.sha256(code).hexdigest()
receipt = {
    'source_sha256': suite['source_after'],
    'verified_main_files': hashes,
    'raw_codeunits': len(current), 'raw_methods': len(new_ids),
    'identities_added': 0, 'identities_removed': 0,
    'product_partition_implemented': False,
    'scope_checks': 255, 'scope_old_policy_failed_checks': 6,
    'old_provisioning_control_failed': True,
    'toolchain_tests_passed': 105,
    'local_cases': 89, 'local_red': 26,
    'database_workflow_proof': False,
    'full_tree_or_G1_proof': False,
}
(artifacts / 'product-scope-proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
