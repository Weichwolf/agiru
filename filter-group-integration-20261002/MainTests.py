import importlib.util
import json
import os
from pathlib import Path
import subprocess
import time

task = Path(__file__).resolve().parent
main = task.parent.parent
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(main)
assert before == json.loads((task / 'artifacts/proof.json').read_text())['own_source_sha256']
started = time.monotonic()
command = ['make', 'tc', 'test', 'JOBS=2']
with (task / 'artifacts/main-tests.log').open('w') as log:
    result = subprocess.run(command, cwd=main,
        env=dict(os.environ, CCACHE_DIR=str(task.parent / 'native-record-context-integration-20261002/ccache')),
        stdout=log, stderr=subprocess.STDOUT)
after = verify.digest(main)
receipt = {'command': command, 'exit': result.returncode, 'elapsed_seconds': time.monotonic() - started,
           'source_before': before, 'source_after': after, 'source_unchanged': before == after}
(task / 'artifacts/main-tests.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert before == after
raise SystemExit(result.returncode)
