import importlib.util
import json
from pathlib import Path
import subprocess

repository = Path('/home/cosmo/Git/agiru')
origin = repository / 'build/page-table-field-20261001/source'
destination = Path(__file__).resolve().parent / 'source'
if destination.exists():
    raise RuntimeError('refusing to overwrite a source image')
spec = importlib.util.spec_from_file_location('verify_snapshot', repository / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(origin)
assert before == '13899bfd7676c8d0f821a4885b36c7ae3d7282887e058d806a4efe7c94ef19ec'
subprocess.run(['git', 'clone', '--shared', '--no-checkout', str(repository), str(destination)], check=True)
verify.copy_source(origin, destination)
assert before == verify.digest(destination) == verify.digest(origin)
(destination.parent / 'source-identity.json').write_text(json.dumps({
    'origin': str(origin), 'source': str(destination), 'parent_sha256': before,
    'production_integrated': False}, indent=2) + '\n')
