import importlib.util
import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
root = task.parent.parent
source = task / 'source'
assert not source.exists()
spec = importlib.util.spec_from_file_location('verify_snapshot', root / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identity = verify.digest(root)
subprocess.run(['git', 'clone', '--shared', '--no-checkout', str(root), str(source)], check=True)
verify.copy_source(root, source)
assert identity == verify.digest(source) == verify.digest(root)
(task / 'artifacts').mkdir()
(task / 'artifacts/origin.json').write_text(json.dumps({'origin_sha256': identity,
    'head': subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()}, indent=2) + '\n')
print(identity)
