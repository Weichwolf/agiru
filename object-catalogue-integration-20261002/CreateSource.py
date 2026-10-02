import importlib.util
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
repository = root.parent.parent
source = root / 'source'
artifacts = root / 'artifacts'
if source.exists():
    raise RuntimeError('refusing to overwrite a source image')
spec = importlib.util.spec_from_file_location('verify_snapshot', repository / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identity = verify.digest(repository)
subprocess.run(['git', 'clone', '--shared', '--no-checkout', str(repository), str(source)], check=True)
verify.copy_source(repository, source)
assert verify.digest(source) == verify.digest(repository) == identity
artifacts.mkdir(exist_ok=True)
(artifacts / 'source-identity.json').write_text(json.dumps({
    'origin': str(repository), 'origin_sha256': identity,
    'head': subprocess.check_output(['git', '-C', str(repository), 'rev-parse', 'HEAD'], text=True).strip(),
    'production_promoted': False}, indent=2) + '\n')
print(json.dumps({'origin_sha256': identity, 'source': str(source)}))
