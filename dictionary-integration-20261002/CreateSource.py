import importlib.util
import json
from pathlib import Path
import subprocess

repository = Path('/home/cosmo/Git/agiru')
root = Path(__file__).resolve().parent
destination = root / 'source'
if destination.exists():
    raise RuntimeError('refusing to overwrite a source image')
spec = importlib.util.spec_from_file_location('verify_snapshot', repository / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(repository)
subprocess.run(['git', 'clone', '--shared', '--no-checkout', str(repository), str(destination)], check=True)
verify.copy_source(repository, destination)
assert before == verify.digest(destination) == verify.digest(repository)
(root / 'artifacts').mkdir(exist_ok=True)
(root / 'artifacts/source-identity.json').write_text(json.dumps({
    'origin': str(repository), 'source': str(destination), 'origin_sha256': before,
    'head': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repository, text=True).strip(),
    'production_integrated': False}, indent=2) + '\n')
