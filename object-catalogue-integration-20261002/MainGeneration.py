import importlib.util
import json
import os
from pathlib import Path
import subprocess
import time

root = Path(__file__).resolve().parent
main = root.parent.parent
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(main)
expected = {path.relative_to(root / 'source').as_posix(): path.read_bytes()
            for path in (root / 'source/apps').rglob('*') if path.is_file()}
assert len(expected) == 24346
inputs = json.loads((root.parent / 'compiler-llvm-20261001/artifacts/inputs.json').read_text())
environment = dict(os.environ, CCACHE_DIR=str(root / 'main-ccache'),
                   AGIRU_BC_SOURCE=str(root.parent / 'compiler-llvm-20261001/bc_source'),
                   AGIRU_BC_REVISION=inputs['bc_revision'],
                   AGIRU_SYSTEM_SYMBOLS=str(root.parent / 'compiler-llvm-20261001/system_symbols'))
started = time.monotonic()
with (root / 'artifacts/main-generation.log').open('w') as log:
    result = subprocess.run(['make', 'transpile', 'JOBS=2'], cwd=main, env=environment, stdout=log, stderr=subprocess.STDOUT)
actual = {path.relative_to(main).as_posix(): path.read_bytes() for path in (main / 'apps').rglob('*') if path.is_file()}
changed = sorted(path for path in expected.keys() & actual.keys() if expected[path] != actual[path])
receipt = {'command': ['make', 'transpile', 'JOBS=2'], 'exit': result.returncode,
           'elapsed_seconds': time.monotonic() - started, 'source_before': before, 'source_after': verify.digest(main),
           'generated_before': len(expected), 'generated_after': len(actual),
           'added': sorted(actual.keys() - expected.keys()), 'removed': sorted(expected.keys() - actual.keys()),
           'changed': changed, 'frozen_inputs': inputs}
(root / 'artifacts/main-generation.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
assert result.returncode == 0 and not receipt['added'] and not receipt['removed'] and not changed
assert receipt['source_after'] == before
