import difflib
import hashlib
import importlib.util
import json
from pathlib import Path
import re

root = Path(__file__).resolve().parent
main = root.parent.parent
source = root / 'source'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
origin = json.loads((artifacts / 'source-identity.json').read_text())
assert verify.digest(main) == origin['origin_sha256']
comparison = json.loads((artifacts / 'consumer-comparison.json').read_text())
assert comparison['sources_unchanged'] and not comparison['losses']
assert comparison['source_hashes']['before'] == verify.digest(main)
assert comparison['source_hashes']['after'] == verify.digest(source)
assert len(comparison['rows']) == 965
lint = json.loads((artifacts / 'draft-lint.json').read_text())
assert not lint['new_findings']
controls = json.loads((artifacts / 'draft-controls.json').read_text())
gate = root / 'candidate/ObjectCatalogueGate.cpp'
assert controls['gate_sha256'] == hashlib.sha256(gate.read_bytes()).hexdigest()
assert controls['sources_unchanged'] and controls['source_hashes'] == comparison['source_hashes']
assert [row['exit'] for row in controls['rows']] == [1, 0]
print('*** Begin Patch')
for name in ('AllObj.h', 'AllObjWithCaption.h', 'AllObjType.h'):
    path = 'include/platform/' + name
    old, new = (main / path).read_text(), (source / path).read_text()
    assert old != new
    difference = list(difflib.unified_diff(old.splitlines(keepends=True), new.splitlines(keepends=True), n=3))
    print('*** Update File: ' + str(main / path))
    for line in difference[2:]:
        print('@@' if line.startswith('@@') else line.rstrip('\n'))
assert not (main / 'test/gate/ObjectCatalogueGate.cpp').exists()
print('*** Add File: ' + str(main / 'test/gate/ObjectCatalogueGate.cpp'))
for line in gate.read_text().splitlines():
    print('+' + line)
print('*** End Patch')
