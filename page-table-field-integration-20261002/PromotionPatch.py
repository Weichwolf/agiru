import importlib.util
import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
origin = json.loads((task / 'artifacts/origin.json').read_text())
assert verify.digest(main) == origin['origin_sha256'], 'main changed after isolation'
tests = json.loads((task / 'artifacts/final-tests.json').read_text())
lint = json.loads((task / 'artifacts/targeted-lint.json').read_text())
assert tests['source_unchanged'] and verify.digest(source) == tests['source_after']
assert lint['sources_unchanged'] and lint['source_hashes'][str(source)] == tests['source_after']
assert not any(row['new_findings'] for row in lint['rows'])
chosen = {
    'include/meta/TableDef.h', 'include/runtime/Storage.h', 'include/type/Option.h',
    'include/platform/PageTableField.h', 'test/gate/PageTableFieldGate.cpp',
    'src/rt/Storage.cpp', 'src/rt/Table.cpp', 'src/rt/Navigate.cpp',
    'src/rt/Selection.cpp', 'src/rt/PlatformTables.cpp',
}
existing = set(verify.files(main))
new = set(verify.files(source))
changed = {str(path) for path in existing | new
           if not (main / path).is_file() or not (source / path).is_file()
           or (main / path).read_bytes() != (source / path).read_bytes()}
assert changed == chosen, changed
print('*** Begin Patch')
for name in sorted(chosen):
    target = main / name
    if not target.exists():
        print('*** Add File: ' + str(target))
        print('\n'.join('+' + line for line in (source / name).read_text().splitlines()))
    else:
        result = subprocess.run(['diff', '-u', str(target), str(source / name)], capture_output=True, text=True)
        assert result.returncode == 1
        print('*** Update File: ' + str(target))
        for line in result.stdout.splitlines()[2:]:
            print('@@' if line.startswith('@@') else line)
print('*** End Patch')
