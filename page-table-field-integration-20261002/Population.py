import hashlib
import importlib.util
import json
from pathlib import Path

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
bc = task.parent / 'compiler-llvm-20261001/bc_source'
spec = importlib.util.spec_from_file_location('ut_manifest', main / 'scripts/ut_manifest.py')
manifest = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manifest)
whole_source_probe_error = None
try:
    manifest.scan(bc)
except ValueError as error:
    whole_source_probe_error = str(error)
tests_root = bc / 'Layers/W1/Tests'
entries = manifest.scan(tests_root)
identities = sorted((entry['id'], entry['name'], method) for entry in entries for method in entry['methods'])
assert len(entries) == 80 and len(identities) == 2310
assert (main / 'scripts/ut_manifest.py').read_bytes() == (source / 'scripts/ut_manifest.py').read_bytes()
assert (main / 'scope.json').read_bytes() == (source / 'scope.json').read_bytes()
assert (main / 'test/slice').read_bytes() == (source / 'test/slice').read_bytes()
old_files = {str(path.relative_to(main / 'apps')): hashlib.sha256(path.read_bytes()).hexdigest()
             for path in (main / 'apps').rglob('*') if path.is_file()}
new_files = {str(path.relative_to(source / 'apps')): hashlib.sha256(path.read_bytes()).hexdigest()
             for path in (source / 'apps').rglob('*') if path.is_file()}
assert old_files == new_files
active = [line.strip() for line in (source / 'test/slice').read_text().splitlines()
          if line.strip() and not line.lstrip().startswith('#')]
assert len(active) == 14212
assert all((source / 'apps' / line).is_file() for line in active)
receipt = {'configured_runner_tests_root': str(tests_root),
           'whole_source_ut_probe_error': whole_source_probe_error,
           'whole_source_ut_population_proved': False,
           'source_codeunits': len(entries), 'source_methods': len(identities), 'identities': identities,
           'source_policy_and_counter_unchanged': True, 'slice_unchanged': True,
           'active_slice_sources': len(active), 'all_active_sources_exist': True,
           'generated_files': len(old_files), 'generated_files_unchanged': True,
           'AL_methods_executed': 0, 'AL_methods_missing': len(identities), 'G1_proved': False}
(task / 'artifacts/population.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key != 'identities'}))
