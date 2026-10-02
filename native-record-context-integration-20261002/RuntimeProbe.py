import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
source = task / 'source'
main = task.parent.parent
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = {str(tree): verify.digest(tree) for tree in (main, source)}
assert len(set(before.values())) == 1
result = subprocess.run([str(artifacts / 'FilterGroupProbe')], cwd=task,
                        capture_output=True, text=True)
(artifacts / 'filter-group-probe.log').write_text(result.stdout + result.stderr)
assert result.returncode == 0
assert result.stdout == 'RecordRef getter: 2 then 0; typed group after 256: 256\n'
receipt = {'source_hashes': before,
    'sources_unchanged': all(verify.digest(Path(tree)) == identity for tree, identity in before.items()),
    'probe_sha256': hashlib.sha256((artifacts / 'FilterGroupProbe.cpp').read_bytes()).hexdigest(),
    'exit': result.returncode, 'uses_database': False,
    'RecordRef_getter_mutates_current_group': True,
    'typed_setter_accepts_group_256': True,
    'setter_return_contract_proved': False, 'runtime_fix_implemented': False}
(artifacts / 'filter-group-probe.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert receipt['sources_unchanged']
