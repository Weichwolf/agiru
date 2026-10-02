import difflib
import hashlib
import importlib.util
import json
from pathlib import Path

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
assert verify.digest(main) == json.loads((artifacts / 'origin.json').read_text())['origin_sha256']
proof = json.loads((artifacts / 'proof.json').read_text())
assert verify.digest(source) == proof['own_source_sha256']
assert proof['toolchain_tests'] == 140 and proof['binding_checks'] == 89
consumers = json.loads((artifacts / 'consumers.json').read_text())
assert not consumers['lost'] and not consumers['new_diagnostics'] and not consumers['new_refusals']
assert json.loads((artifacts / 'generation.json').read_text())['compiler_inputs_unchanged']
expected = {'src/gen/BodyWriter.cpp', 'src/gen/CodeunitWriter.cpp', 'src/gen/CodeunitWriter.h',
            'test/gate/GenSourceBindingGate.cpp', 'test/toolchain.py',
            'test/page-record-binding/apps.json', 'test/page-record-binding/scope.json',
            'test/page-record-binding/Consumer.cpp.in',
            'test/page-record-binding/al/fixture/RecordBinding.Page.al'}

def files(tree):
    return {str(relative): hashlib.sha256((tree / relative).read_bytes()).hexdigest()
            for relative in verify.files(tree) if relative.parts[0] != 'apps'}

old, new = files(main), files(source)
changed = {path for path in old.keys() & new.keys() if old[path] != new[path]}
added, removed = new.keys() - old.keys(), old.keys() - new.keys()
assert not removed and changed | added == expected
patches = []
for path in sorted(expected):
    text = (source / path).read_text()
    if path in added:
        patches.append('*** Add File: ' + str(main / path) + '\n' +
                       '\n'.join('+' + line for line in text.splitlines()) + '\n')
    else:
        prior = (main / path).read_text()
        diff = ''.join(difflib.unified_diff(prior.splitlines(True), text.splitlines(True), n=3))
        lines = ['@@' if line.startswith('@@') else line for line in diff.splitlines()[2:]]
        patches.append('*** Update File: ' + str(main / path) + '\n' + '\n'.join(lines) + '\n')
print('*** Begin Patch\n' + ''.join(patches) + '*** End Patch')
