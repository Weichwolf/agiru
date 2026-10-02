import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parent
main = root.parent.parent
artifacts = root / 'artifacts'
report = json.loads((root / 'source/build/scope-inventory.json').read_text())
suite = json.loads((artifacts / 'raw-inventory-final-tests.json').read_text())
census = json.loads((artifacts / 'raw-inventory-final-census.json').read_text())
assert suite['source_unchanged'] and census['source_unchanged']
assert suite['source_after'] == census['source_before'] == census['source_after']
assert suite['exit'] == census['exit'] == 2
log = (artifacts / 'raw-inventory-final-tests.log').read_text()
assert 'Ran 120 tests' in log and '\nOK\n' in log
assert 'test: 89 case(s), 26 red' in log
summary = report['summary']
assert summary['files'] == 36697 and summary['objects'] == 36605
assert summary['test_codeunits'] == 4137
assert summary['test_methods'] == summary['test_attributes'] == summary['raw_test_attributes'] == 112055
assert len(report['errors']) == summary['unmeasured_files'] == 1
assert report['errors'][0]['source'] == 'Layers/GB/BaseApp/Sales/History/SalesShipment.Report.al'
assert summary['outside_configured_app_files'] == 25018
assert report['source_revision'] == 'a9ea4d84534cebba852c44bf0f841c2ea149de4e'
current = json.loads((artifacts / 'raw-manifest-current.json').read_text())
previous = json.loads((main / 'build/chart-20261001/artifacts/ut-milestone.log.manifest.json').read_text())


def identities(entries):
    return {(item['id'], item['name'], method)
            for item in entries for method in item['methods']}


old_ids, current_ids = identities(previous), identities(current)
milestone = [item for item in report['objects'] if item['kind'] == 'codeunit'
             and item['test_subtype'] and item['source'].startswith('Layers/W1/Tests/')
             and item['name'].endswith((' UT', '-UT', '.UT'))]
assert old_ids == current_ids == identities(milestone)
assert len(current) == len(milestone) == 80 and len(current_ids) == 2310
gains = [item for item in report['objects'] if item['source'] in (
    'Layers/W1/Tests/SCM-Warehouse/SCMWarehouseShippingIII.Codeunit.al',
    'Layers/W1/Tests/VAT/ERMVATServCharge.Codeunit.al')]
assert len(gains) == 2 and sum(len(item['methods']) for item in gains) == 94
w1 = [item for item in report['objects'] if item['kind'] == 'codeunit'
      and item['test_subtype'] and item['source'].startswith('Layers/W1/Tests/')]
hashes = {}
for name in ('Makefile', 'scripts/scope_inventory.py', 'scripts/ut_manifest.py', 'test/toolchain.py'):
    value = (main / name).read_bytes()
    assert value == (root / 'source' / name).read_bytes(), name
    hashes[name] = hashlib.sha256(value).hexdigest()
old_controls = (artifacts / 'raw-manifest-old-controls.log').read_text()
assert 'FAILED (failures=1, errors=3)' in old_controls
assert 'test_make_inventory_has_no_build_or_database_dependency_and_preserves_exit' in (
    artifacts / 'raw-inventory-old-make.log').read_text()
receipt = {
    'verified_source_sha256': suite['source_after'], 'verified_main_files': hashes,
    'raw_source_sha256': report['source_sha256'], 'raw_summary': summary,
    'raw_inventory_errors': report['errors'], 'inventory_exit': census['exit'],
    'raw_w1_test_codeunits': len(w1), 'raw_w1_test_methods': sum(len(item['methods']) for item in w1),
    'raw_test_gains': [{'source': item['source'], 'id': item['id'], 'name': item['name'],
                       'methods': len(item['methods'])} for item in gains],
    'ut_codeunits': 80, 'ut_methods': 2310, 'ut_identity_gains': 0, 'ut_identity_losses': 0,
    'independent_inventory_and_manifest_agree': True,
    'toolchain_green': 120, 'local_cases': 89, 'local_red': 26,
    'product_partition_implemented': False, 'full_tree_or_G1_proved': False,
}
(artifacts / 'raw-inventory-proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
