import difflib
import importlib.util
import json
from pathlib import Path

root = Path(__file__).resolve().parent
repository = root.parent.parent
source = root / 'source'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
origin = json.loads((artifacts / 'source-identity.json').read_text())
assert verify.digest(repository) == origin['origin_sha256'], 'main inputs changed; reconcile before promotion'
assert verify.digest(source) == '0581bb5dbba3e123038eb63c4d250d514c92bcc86a96e242a16ca0e5636b383c'
lint = json.loads((artifacts / 'targeted-lint.json').read_text())
assert lint['sources_unchanged'] and not any(row['new_findings'] for row in lint['rows'])
for name in ('body-comparison', 'consumer-comparison'):
    receipt = json.loads((artifacts / (name + '.json')).read_text())
    assert not receipt['compile_losses']
for name in ('lint', 'lint-full', 'post-generation-tests'):
    receipt = json.loads((artifacts / (name + '.json')).read_text())
    assert receipt['source_unchanged'] and receipt['source_before'] == verify.digest(source)
units = ['include/dotnet/Generic.h', 'src/net/Generic.cpp', 'src/gen/Door.cpp',
         'src/gen/BodyWriter.cpp', 'include/dotnet/DesignerFieldProperty.h',
         'include/dotnet/DesignerFieldType.h', 'test/gate/DictionaryGate.cpp',
         'test/gate/DesignerConstantsGate.cpp', 'test/gate/GenReceiverGate.cpp', 'test/slice']
parts = ['*** Begin Patch\n']
for unit in units:
    old_path = repository / unit
    new = (source / unit).read_text()
    if old_path.exists():
        old = old_path.read_text()
        assert old != new, unit
        diff = list(difflib.unified_diff(old.splitlines(True), new.splitlines(True)))
        parts += ['*** Update File: ' + str(old_path) + '\n', *diff[2:]]
    else:
        parts += ['*** Add File: ' + str(old_path) + '\n',
                  ''.join('+' + line + '\n' for line in new.splitlines())]
parts.append('*** End Patch\n')
print(''.join(parts), end='')
