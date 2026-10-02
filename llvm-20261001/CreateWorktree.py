import importlib.util
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parents[2]
destination = Path(__file__).resolve().parent / 'source'
if destination.exists():
    raise SystemExit('refusing to overwrite a source image')
spec = importlib.util.spec_from_file_location('verify_snapshot', root / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(root)
subprocess.run(['git', 'clone', '--shared', '--no-checkout', str(root), str(destination)], check=True)
verify.copy_source(root, destination)
assert verify.digest(destination) == verify.digest(root) == before, 'source moved during copy'
receipt = {'origin': str(root), 'source': str(destination), 'source_sha256': before,
           'git_head': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()}
(destination.parent / 'source-identity.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
