from pathlib import Path
import importlib.util
import json
import subprocess

origin = Path('/home/cosmo/Git/agiru/build/privacy-binding-20261001/source')
repository = Path('/home/cosmo/Git/agiru')
destination = Path(__file__).resolve().parent / 'source'
if destination.exists():
    raise RuntimeError('refusing to overwrite a preserved source image')
spec = importlib.util.spec_from_file_location('verify_snapshot', origin / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(origin)
head = subprocess.check_output(['git', '-C', str(origin), 'rev-parse', 'HEAD'], text=True).strip()
subprocess.run(['git', 'clone', '--shared', '--no-checkout', str(repository), str(destination)],
               check=True)
verify.copy_source(origin, destination)
after = verify.digest(destination)
if before != after or verify.digest(origin) != before:
    raise RuntimeError('source changed during preservation')
receipt = {'origin': str(origin), 'destination': str(destination), 'git_head': head,
           'base_source_sha256': after, 'origin_modified': False,
           'working_copy_not_frozen_integration': True, 'production_integrated': False}
(destination.parent / 'source-identity.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt, indent=2))
