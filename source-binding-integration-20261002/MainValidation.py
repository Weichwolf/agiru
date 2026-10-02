import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import time

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
tested = json.loads((artifacts / 'final-tests.json').read_text())
assert verify.digest(source) == tested['source_after']
inputs = json.loads((task.parent / 'compiler-llvm-20261001/artifacts/inputs.json').read_text())
environment = dict(os.environ, CCACHE_DIR=str(task / 'main-ccache'),
    AGIRU_AL_SOURCE=str(task.parent / 'compiler-llvm-20261001/bc_source/Layers/W1/BaseApp'),
    AGIRU_BC_SOURCE=str(task.parent / 'compiler-llvm-20261001/bc_source'),
    AGIRU_BC_REVISION=inputs['bc_revision'],
    AGIRU_SYSTEM_SYMBOLS=str(task.parent / 'compiler-llvm-20261001/system_symbols'))

def handwritten(tree):
    return {str(path): hashlib.sha256((tree / path).read_bytes()).hexdigest()
        for path in verify.files(tree) if path.parts[0] != 'apps'}

assert handwritten(main) == handwritten(source)
before_inputs = handwritten(main)
before = verify.digest(main)
started = time.monotonic()
command = ['make', 'transpile', 'JOBS=2']
with (artifacts / 'main-generation.log').open('w') as log:
    result = subprocess.run(command, cwd=main, env=environment, stdout=log, stderr=subprocess.STDOUT)
after = verify.digest(main)
receipt = dict(command=command, exit=result.returncode, elapsed_seconds=time.monotonic() - started,
    source_before=before, source_after=after,
    handwritten_inputs_unchanged=before_inputs == handwritten(main),
    generated_image_equals_reviewed_image=after == verify.digest(source), frozen_inputs=inputs,
    native_source_input_consumed=False, G1_proved=False)
(artifacts / 'main-generation.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
assert result.returncode == 0 and receipt['handwritten_inputs_unchanged']
assert receipt['generated_image_equals_reviewed_image']
started = time.monotonic()
command = ['make', 'tc', 'test', 'JOBS=2']
with (artifacts / 'main-tests.log').open('w') as log:
    result = subprocess.run(command, cwd=main, env=environment, stdout=log, stderr=subprocess.STDOUT)
final = verify.digest(main)
receipt = dict(command=command, exit=result.returncode, elapsed_seconds=time.monotonic() - started,
    source_before=after, source_after=final, source_unchanged=after == final,
    complete_image_equals_reviewed_tested_image=final == tested['source_after'],
    production_promoted=True, G1_proved=False)
(artifacts / 'main-tests.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
assert receipt['source_unchanged'] and receipt['complete_image_equals_reviewed_tested_image']
raise SystemExit(result.returncode)
