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
identities = {str(tree): verify.digest(tree) for tree in (main, source)}
assert identities[str(main)] == json.loads((artifacts / 'origin.json').read_text())['origin_sha256']
pattern = re.compile(r'^([\w -]+): (\d+) check\(s\), (\d+) red$', re.M)

def results(path):
    text = path.read_text()
    measured = {match[1]: (int(match[2]), int(match[3])) for match in pattern.finditer(text)}
    exceptions = set(re.findall(r'^FAIL  an exception left ([\w -]+)$', text, re.M))
    return text, measured, exceptions, exceptions | {name for name, (_, red) in measured.items() if red}

old_text, old, old_exceptions, old_failed = results(artifacts / 'before-tests.log')
new_text, new, new_exceptions, new_failed = results(artifacts / 'stable-tests.log')
assert old_failed == new_failed and len(new_failed) == 26
assert old_exceptions == new_exceptions and set(new) - set(old) == {'FilterGroup'}
assert all(new[name] == result for name, result in old.items())
assert new['FilterGroup'] == (137, 0)
assert 'test: 96 case(s), 26 red' in old_text
assert 'test: 97 case(s), 26 red' in new_text and 'Ran 140 tests' in new_text and '\nOK\n' in new_text
assert not re.search(r'skipped|missing executable|unknown exception', new_text, re.I)
assert new_text.count('Connection:') == old_text.count('Connection:') == 26
assert json.loads((artifacts / 'stable-tests.json').read_text())['source_unchanged']

def python_tests(tree):
    parsed = ast.parse((tree / 'test/toolchain.py').read_text())
    return {declaration.name + '.' + method.name: ast.dump(method, include_attributes=False)
        for declaration in parsed.body if isinstance(declaration, ast.ClassDef)
        for method in declaration.body if isinstance(method, ast.FunctionDef) and method.name.startswith('test_')}

assert python_tests(main) == python_tests(source) and len(python_tests(source)) == 140
old_files = {str(relative): hashlib.sha256((main / relative).read_bytes()).hexdigest() for relative in verify.files(main)}
new_files = {str(relative): hashlib.sha256((source / relative).read_bytes()).hexdigest() for relative in verify.files(source)}
expected = {'include/runtime/RecordState.h', 'include/runtime/Record.h', 'include/runtime/Table.h',
    'include/runtime/RecordRef.h', 'src/rt/Temporary.cpp', 'src/rt/RecordRef.cpp',
    'test/gate/FilterGroupGate.cpp', 'test/page-record-binding/Consumer.cpp.in',
    'test/page-record-binding/al/fixture/RecordBinding.Page.al'}
changed = {path for path in old_files.keys() & new_files.keys() if old_files[path] != new_files[path]}
added, removed = new_files.keys() - old_files.keys(), old_files.keys() - new_files.keys()
assert not removed and changed | added == expected and added == {'test/gate/FilterGroupGate.cpp'}
generated = {path: value for path, value in old_files.items() if path.startswith('apps/')}
assert len(generated) == 24346 and generated == {path: value for path, value in new_files.items() if path.startswith('apps/')}
slice_paths = [line for line in (source / 'test/slice').read_text().splitlines() if line and not line.startswith('#')]
assert len(slice_paths) == 14212 and all((source / 'apps' / path).exists() for path in slice_paths)
previous = json.loads((task.parent / 'native-record-context-integration-20261002/artifacts/census-comparison.json').read_text())
for path in ('scripts/scope_inventory.py', 'scope.json', 'apps.json', 'scripts/ut_manifest.py'):
    assert (main / path).read_bytes() == (source / path).read_bytes() == (task.parent / 'native-record-context-integration-20261002/source' / path).read_bytes()
bc = task.parent / 'compiler-llvm-20261001/bc_source'
assert verify.digest(bc) == previous['bc_input_sha256']
spec = importlib.util.spec_from_file_location('ut_manifest', source / 'scripts/ut_manifest.py')
scanner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(scanner)
manifest = scanner.scan(bc / 'Layers/W1/Tests')
assert len(manifest) == 80 and sum(len(entry['methods']) for entry in manifest) == 2310
(artifacts / 'ut-manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
methods = [dict(codeunit=entry['id'], name=entry['name'], method=method, status='missing', reason='No current complete linked AL runner or reachable gate database.') for entry in manifest for method in entry['methods']]
(artifacts / 'ut-statuses.json').write_text(json.dumps(methods, indent=2) + '\n')
lint = json.loads((artifacts / 'stable-targeted-lint.json').read_text())
controls = json.loads((artifacts / 'stable-controls.json').read_text())
consumers = json.loads((artifacts / 'consumers.json').read_text())
assert lint['sources_unchanged'] and not any(row['new_findings'] for row in lint['rows'])
assert controls['source_unchanged'] and all(row['red'] > 0 for row in controls['rows'])
assert consumers['sources_unchanged'] and not consumers['lost'] and not consumers['new_diagnostics']
elf = []
for name in ('libagiru_al.so', 'libagiru_net.so', 'libagiru_db.so', 'libagiru_gen.so', 'libagiru_rt.so', 'agirutc'):
    path = source / 'build' / name
    result = subprocess.run(['readelf', '-d', str(path)], capture_output=True, text=True, check=True)
    assert 'libstdc++' not in result.stdout and 'libgcc_s' not in result.stdout
    elf.append({'name': name, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                'needed': re.findall(r'\(NEEDED\).*?\[([^]]+)\]', result.stdout)})
receipt = dict(main_source_sha256=identities[str(main)], own_source_sha256=identities[str(source)],
    sources_unchanged=all(verify.digest(Path(tree)) == value for tree, value in identities.items()),
    changed=sorted(changed), added=sorted(added), removed=sorted(removed),
    local_cases_before=96, local_cases=97, database_failures=26, unchanged_failure_identities=sorted(new_failed),
    all_previous_cpp_counts_and_statuses_retained=True, filter_group_checks=137, toolchain_tests=140,
    old_python_test_bodies_unchanged=True, generated_paths=24346, generated_bytes_unchanged=True,
    slice_sources=14212, slice_unchanged=True, configured_UT_objects=80, configured_UT_methods=2310,
    configured_AL_methods_executed=0, configured_AL_methods_missing=2310,
    census_reuse_basis='Byte-identical scanner, apps/scope policy and unchanged frozen AL source; this is not a new complete census or an executable full-AL population.',
    census_summary=previous['summary'], census_errors=previous['errors'],
    bc_input_sha256=previous['bc_input_sha256'], bc_revision=previous['bc_revision'],
    consumer_units=len(consumers['units']), consumer_before_green=consumers['before_green'],
    consumer_after_green=consumers['after_green'], elf=elf,
    production_promoted=False, SQL_or_BC_setter_return_or_full_tree_or_G1_proved=False)
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key != 'elf'}))
assert receipt['sources_unchanged']
