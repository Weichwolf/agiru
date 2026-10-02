import hashlib
import importlib.util
import json
from pathlib import Path
import re

root = Path(__file__).resolve().parent
main = root.parent.parent
source = root / 'source'
artifacts = root / 'artifacts'


def receipt(name):
    value = json.loads((artifacts / (name + '.json')).read_text())
    assert value['source_unchanged'], name
    return value


suite = receipt('canonical-scope-refactored-tests')
census = receipt('canonical-scope-refactored-census')
generation = receipt('canonical-scope-refactored-generation')
lint = receipt('canonical-scope-refactored-lint-driver')
assert len({item['source_after'] for item in (suite, census, generation, lint)}) == 1
assert suite['exit'] == census['exit'] == lint['exit'] == 2
assert generation['exit'] == 0 and generation['excluded_object_absent']
log = (artifacts / 'canonical-scope-refactored-tests.log').read_text()
assert 'Ran 127 tests' in log and '\nOK\n' in log
assert 'GenScope: 294 check(s), 0 red' in log and 'test: 89 case(s), 26 red' in log

old = json.loads((source / 'build/scope-inventory.json').read_text())
report = json.loads((source / 'build/canonical-scope-refactored/scope-inventory.json').read_text())
assert report['source_sha256'] == old['source_sha256']
assert report['source_revision'] == 'a9ea4d84534cebba852c44bf0f841c2ea149de4e'
for key, value in old['summary'].items():
    assert report['summary'][key] == value, key


def raw_objects(items):
    return [{key: value for key, value in item.items()
             if key not in ('namespace_selected', 'product_exclusion_reason')} for item in items]


assert raw_objects(old['objects']) == raw_objects(report['objects'])
assert [(item['source'], item['sha256']) for item in old['files']] == [
    (item['source'], item['sha256']) for item in report['files']]
assert len(report['errors']) == report['summary']['unmeasured_files'] == 1
assert report['errors'][0]['source'] == 'Layers/GB/BaseApp/Sales/History/SalesShipment.Report.al'
excluded = [item for item in report['objects'] if item['product_exclusion_reason']]
assert len(excluded) == report['summary']['product_excluded_objects'] == 1
assert excluded[0]['id'] == 138073 and len(excluded[0]['methods']) == 15
assert report['summary']['product_excluded_test_methods'] == 15
assert report['summary']['product_required_test_methods'] == 112040

spec = importlib.util.spec_from_file_location('ut_manifest', source / 'scripts/ut_manifest.py')
manifest = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manifest)
current = manifest.scan(Path('/home/cosmo/Git/BCApps/src/Layers/W1/Tests'))
previous = json.loads((artifacts / 'raw-manifest-current.json').read_text())


def identities(entries):
    return {(item['id'], item['name'], method) for item in entries for method in item['methods']}


milestone = [item for item in report['objects'] if item['kind'] == 'codeunit'
             and item['test_subtype'] and item['source'].startswith('Layers/W1/Tests/')
             and item['name'].endswith((' UT', '-UT', '.UT'))]
assert identities(current) == identities(previous) == identities(milestone)
assert len(current) == len(milestone) == 80 and len(identities(current)) == 2310
assert not any(item['product_exclusion_reason'] for item in milestone)
(artifacts / 'canonical-scope-manifest.json').write_text(json.dumps(current, indent=2) + '\n')

hashes = {}
for name in ('scope.json', 'src/gen/Apps.h', 'src/gen/Apps.cpp', 'src/tc/Main.cpp',
             'test/gate/GenScopeGate.cpp', 'scripts/scope_inventory.py',
             'scripts/ut_manifest.py', 'test/toolchain.py'):
    value = (main / name).read_bytes()
    assert value == (source / name).read_bytes(), name
    hashes[name] = hashlib.sha256(value).hexdigest()
assert not (main / 'src/gen/scope.json').exists() and not (source / 'src/gen/scope.json').exists()
assert report['scope_sha256'] == hashes['scope.json']
generated = {path.name for path in (source / 'build/canonical-scope-refactored-generation-generated').rglob('*')
             if path.is_file()}
assert {'O365TrialBalance.h', 'O365TrialBalance.cpp',
        'GraphCollectionMgtItem.h', 'GraphCollectionMgtItem.cpp'} <= generated
negative = (artifacts / 'canonical-scope-old-controls.log').read_text()
assert 'Ran 5 tests' in negative and 'FAILED (failures=13)' in negative
negative_counting = (artifacts / 'canonical-scope-counting-old-controls.log').read_text()
assert 'Ran 2 tests' in negative_counting and 'FAILED (failures=1, errors=1)' in negative_counting
lint_text = (source / 'build/lint/targeted.log').read_text()
assert "function 'Scan' has cognitive complexity of 93" in lint_text
(artifacts / 'canonical-scope-refactored-lint-driver-diagnostics.log').write_text(lint_text)
proof = {
    'verified_source_sha256': suite['source_after'], 'verified_main_files': hashes,
    'raw_source_sha256': report['source_sha256'], 'raw_summary': report['summary'],
    'raw_object_identity_gains': 0, 'raw_object_identity_losses': 0,
    'excluded_objects': excluded, 'raw_inventory_errors': report['errors'],
    'ut_codeunits': 80, 'ut_methods': 2310, 'ut_excluded_methods': 0,
    'ut_identity_gains': 0, 'ut_identity_losses': 0,
    'scope_checks_green': 294, 'toolchain_tests_green': 127, 'toolchain_skipped': 0,
    'old_policy_control_methods': 5, 'old_policy_assertion_failures': 13,
    'old_counting_path_control_methods_red': 2, 'local_cases': 89, 'local_cases_red': 26,
    'generation': generation, 'generic_erp_helpers_retained': True,
    'apps_cpp_targeted_lint_exit': receipt('canonical-scope-lint-apps')['exit'],
    'driver_targeted_lint_exit': lint['exit'],
    'driver_diagnostic_count': len(re.findall(r': error: ', lint_text)),
    'runner_partition_complete': False, 'product_dependency_closure_complete': False,
    'full_tree_or_G1_proved': False,
}
(artifacts / 'canonical-scope-proof.json').write_text(json.dumps(proof, indent=2) + '\n')
print(json.dumps({key: proof[key] for key in ('verified_source_sha256', 'ut_codeunits', 'ut_methods',
                 'scope_checks_green', 'toolchain_tests_green', 'local_cases_red',
                 'driver_diagnostic_count', 'full_tree_or_G1_proved')}))
