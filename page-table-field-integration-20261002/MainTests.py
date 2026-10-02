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
tested = json.loads((task / 'artifacts/final-tests.json').read_text())
assert before == tested['source_after'] == verify.digest(task / 'source')
inputs = json.loads((task.parent / 'compiler-llvm-20261001/artifacts/inputs.json').read_text())
environment = dict(os.environ, CCACHE_DIR=str(task / 'main-ccache'),
                   AGIRU_AL_SOURCE=str(task.parent / 'compiler-llvm-20261001/bc_source/Layers/W1/BaseApp'),
                   AGIRU_BC_SOURCE=str(task.parent / 'compiler-llvm-20261001/bc_source'),
                   AGIRU_BC_REVISION=inputs['bc_revision'],
                   AGIRU_SYSTEM_SYMBOLS=str(task.parent / 'compiler-llvm-20261001/system_symbols'))
started = time.monotonic()
command = ['make', 'tc', 'test', 'JOBS=2']
with (task / 'artifacts/main-tests.log').open('w') as log:
    result = subprocess.run(command, cwd=main, env=environment, stdout=log, stderr=subprocess.STDOUT)
after = verify.digest(main)
receipt = dict(command=command, exit=result.returncode, elapsed_seconds=time.monotonic() - started,
               source_before=before, source_after=after, source_unchanged=before == after,
               compiler_source_equals_isolated_test_image=before == verify.digest(task / 'source'),
               complete_G1_proved=False, production_promoted=True)
(task / 'artifacts/main-tests.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert before == after
raise SystemExit(result.returncode)
