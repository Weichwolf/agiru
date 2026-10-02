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
proof = json.loads((artifacts / 'proof.json').read_text())
assert verify.digest(main) == proof['main_source_sha256']
assert verify.digest(source) == proof['own_source_sha256']
assert proof['sources_unchanged'] and proof['filter_group_checks'] == 137
assert proof['all_previous_cpp_counts_and_statuses_retained']
patches = []
for path in proof['changed'] + proof['added']:
    text = (source / path).read_text()
    if path in proof['added']:
        patches.append('*** Add File: ' + str(main / path) + '\n' +
                       '\n'.join('+' + line for line in text.splitlines()) + '\n')
    else:
        prior = (main / path).read_text()
        diff = ''.join(difflib.unified_diff(prior.splitlines(True), text.splitlines(True), n=3))
        lines = ['@@' if line.startswith('@@') else line for line in diff.splitlines()[2:]]
        patches.append('*** Update File: ' + str(main / path) + '\n' + '\n'.join(lines) + '\n')
print('*** Begin Patch\n' + ''.join(patches) + '*** End Patch')
