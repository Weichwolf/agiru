import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import time

root = Path(__file__).resolve().parent
source = root.parent.parent
artifacts = root / 'artifacts'
compiler = root.parent / 'compiler-llvm-20261001'
inputs = json.loads((compiler / 'artifacts/inputs.json').read_text())
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
label, *targets = sys.argv[1:]
assert targets
environment = dict(os.environ, CCACHE_DIR=str(root / 'ccache'),
                   AGIRU_BC_SOURCE=str(compiler / 'bc_source'),
                   AGIRU_BC_REVISION=inputs['bc_revision'],
                   AGIRU_AL_SOURCE=str(compiler / 'bc_source/Layers/W1/BaseApp'),
                   AGIRU_SYSTEM_SYMBOLS=str(compiler / 'system_symbols'))
before = verify.digest(source)
command = ['make', *targets, 'JOBS=' + os.environ.get('AGIRU_PROOF_JOBS', '6')]
started = time.monotonic()
with (artifacts / (label + '.log')).open('w') as output:
    result = subprocess.run(command, cwd=source, env=environment,
                            stdout=output, stderr=subprocess.STDOUT)
receipt = {'command': command, 'exit': result.returncode,
           'elapsed_seconds': time.monotonic() - started,
           'source_before': before, 'source_after': verify.digest(source), 'frozen_inputs': inputs}
(artifacts / (label + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
raise SystemExit(result.returncode)
