import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import time

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
artifacts.mkdir(exist_ok=True)
name, *targets = sys.argv[1:]
assert targets and 'gcc' not in targets
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
inputs = json.loads((root.parent / 'compiler-llvm-20261001/artifacts/inputs.json').read_text())
environment = dict(os.environ, CCACHE_DIR=str(root / 'ccache'),
                   AGIRU_BC_SOURCE=str(root.parent / 'compiler-llvm-20261001/bc_source'),
                   AGIRU_BC_REVISION=inputs['bc_revision'],
                   AGIRU_SYSTEM_SYMBOLS=str(root.parent / 'compiler-llvm-20261001/system_symbols'))
command = ['make', *targets, 'JOBS=' + os.environ.get('AGIRU_PROOF_JOBS', '6')]
before = verify.digest(source)
started = time.monotonic()
with (artifacts / (name + '.log')).open('w') as log:
    result = subprocess.run(command, cwd=source, env=environment, stdout=log, stderr=subprocess.STDOUT)
after = verify.digest(source)
receipt = {'command': command, 'exit': result.returncode, 'elapsed_seconds': time.monotonic() - started,
           'source_before': before, 'source_after': after, 'source_unchanged': before == after,
           'frozen_inputs': inputs}
(artifacts / (name + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert before == after
raise SystemExit(result.returncode)
