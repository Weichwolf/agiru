import importlib.util
import json
from pathlib import Path
import subprocess

repository = Path('/home/cosmo/Git/agiru')
origin = repository / 'build/page-source-binding-20261001/source'
destination = Path(__file__).resolve().parent / 'source'
if destination.exists():
    raise RuntimeError('refusing to overwrite a source image')
spec = importlib.util.spec_from_file_location('verify_snapshot', repository / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(origin)
assert before == '6167d4c0faf2acc5b2a001e832d45c42beca0184461edba3c617272c35c17d2d'
subprocess.run(['git', 'clone', '--shared', '--no-checkout', str(repository), str(destination)], check=True)
verify.copy_source(origin, destination)
assert before == verify.digest(destination) == verify.digest(origin)
(destination.parent / 'source-identity.json').write_text(json.dumps({
    'origin': str(origin), 'source': str(destination), 'parent_sha256': before,
    'production_integrated': False}, indent=2) + '\n')
