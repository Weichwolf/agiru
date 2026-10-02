import importlib.util
import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
repository = task.parent.parent
source = task / 'source'
if source.exists():
    raise RuntimeError('refusing to replace an existing source image')
spec = importlib.util.spec_from_file_location('verify_snapshot', repository / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identity = verify.digest(repository)
subprocess.run(['git', 'clone', '--shared', '--no-checkout', str(repository), str(source)], check=True)
verify.copy_source(repository, source)
assert identity == verify.digest(repository) == verify.digest(source)
artifacts = task / 'artifacts'
artifacts.mkdir()
(artifacts / 'origin.json').write_text(json.dumps({
    'origin_sha256': identity,
    'head': subprocess.check_output(['git', '-C', str(repository), 'rev-parse', 'HEAD'], text=True).strip(),
    'production_promoted': False,
}, indent=2) + '\n')
print(identity)
