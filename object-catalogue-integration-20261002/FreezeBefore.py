import importlib.util
import json
from pathlib import Path
import subprocess
import hashlib

root = Path(__file__).resolve().parent
main = root.parent.parent
destination = root / 'before-source'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
expected = json.loads((root / 'artifacts/source-identity.json').read_text())['origin_sha256']
assert verify.digest(main) == expected
if not destination.exists():
    verify.copy_source(main, destination)
assert verify.digest(destination) == expected and verify.digest(main) == expected
(destination / 'build').mkdir(exist_ok=True)
libraries = {}
for name in ('libagiru_al.so', 'libagiru_net.so', 'libagiru_db.so', 'libagiru_gen.so', 'libagiru_rt.so'):
    original, frozen = main / 'build' / name, destination / 'build' / name
    assert original.is_file() and not original.is_symlink(), original
    subprocess.run(['cp', '-a', '--reflink=auto', str(original), str(frozen)], check=True)
    before = hashlib.sha256(original.read_bytes()).hexdigest()
    assert hashlib.sha256(frozen.read_bytes()).hexdigest() == before
    libraries[name] = before
receipt = {'origin': str(main), 'before_source': str(destination), 'source_sha256': expected,
           'matching_before_runtime_library_sha256': libraries,
           'original_source_unchanged': verify.digest(main) == expected}
(root / 'artifacts/before-frozen.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
