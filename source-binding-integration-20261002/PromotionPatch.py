import importlib.util
import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
before = task / 'before-source'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
origin = json.loads((task / 'artifacts/origin.json').read_text())
assert verify.digest(main) == verify.digest(before) == origin['origin_sha256']
tests = json.loads((task / 'artifacts/final-tests.json').read_text())
lint = json.loads((task / 'artifacts/binding-analysis.json').read_text())
consumers = json.loads((task / 'artifacts/consumers.json').read_text())
analysis = json.loads((task / 'artifacts/final-analysis.json').read_text())
assert verify.digest(source) == tests['source_after'] == lint['source_after']
assert tests['source_unchanged'] and lint['source_unchanged'] and lint['exit'] == 0
assert analysis['source_unchanged'] and analysis['source_sha256'] == tests['source_after']
assert not any(row['new_findings'] for row in analysis['rows'])
assert consumers['sources_unchanged'] and not consumers['lost']
assert all(not row['added'] for row in consumers['diagnostic_changes'])
chosen = {'src/gen/TableWriter.h', 'src/gen/TableWriter.cpp', 'src/gen/CodeunitWriter.cpp',
    'src/gen/BodyWriter.cpp', 'src/tc/Main.cpp', 'test/gate/GenSourceBindingGate.cpp', 'test/toolchain.py'}
chosen.update(str(path.relative_to(source)) for path in (source / 'test/source-binding').rglob('*') if path.is_file())
existing = set(verify.files(main))
new = set(verify.files(source))
changed = {str(path) for path in existing | new if path.parts[0] != 'apps'
    and (not (main / path).is_file() or not (source / path).is_file()
        or (main / path).read_bytes() != (source / path).read_bytes())}
assert changed == chosen and len(chosen) == 13, changed
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
