import importlib.util
import json
from pathlib import Path
import shutil

task = Path(__file__).resolve().parent
main = task.parent.parent
before = task / 'before-source'
assert not before.exists()
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
origin = json.loads((task / 'artifacts/origin.json').read_text())['origin_sha256']
assert verify.digest(main) == origin
verify.copy_source(main, before)
assert verify.digest(before) == verify.digest(main) == origin
(before / 'build').mkdir()
for name in ('agirutc', 'libagiru_al.so', 'libagiru_gen.so', 'libagiru_net.so', 'libagiru_db.so', 'libagiru_rt.so'):
    shutil.copy2(main / 'build' / name, before / 'build' / name)
print(origin)
