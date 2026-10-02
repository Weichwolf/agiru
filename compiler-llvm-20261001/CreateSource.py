import importlib.util
import json
from pathlib import Path
import subprocess

repository = Path('/home/cosmo/Git/agiru')
origin = repository / 'build/chart-20261001/source'
destination = Path(__file__).resolve().parent / 'source'
if destination.exists():
    raise RuntimeError('refusing to overwrite a source image')
spec = importlib.util.spec_from_file_location('verify_snapshot', repository / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(origin)
assert before == '7d8acdccca3fa0f2a9b031199ac6c4522915d5f686b29de15e638c47a0098cb9'
subprocess.run(['git', 'clone', '--shared', '--no-checkout', str(repository), str(destination)], check=True)
verify.copy_source(origin, destination)
assert before == verify.digest(destination) == verify.digest(origin)
receipt = {'origin': str(origin), 'source': str(destination), 'parent_sha256': before,
           'git_head': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repository, text=True).strip(),
           'production_integrated': False}
(destination.parent / 'source-identity.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
