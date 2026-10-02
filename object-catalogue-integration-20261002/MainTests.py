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
for name in ('AllObj.h', 'AllObjWithCaption.h', 'AllObjType.h'):
    assert (main / 'include/platform' / name).read_bytes() == (root / 'source/include/platform' / name).read_bytes()
assert (main / 'test/gate/ObjectCatalogueGate.cpp').read_bytes() == (root / 'candidate/ObjectCatalogueGate.cpp').read_bytes()
before = verify.digest(main)
inputs = json.loads((root.parent / 'compiler-llvm-20261001/artifacts/inputs.json').read_text())
environment = dict(os.environ, CCACHE_DIR=str(root / 'main-ccache'),
                   AGIRU_AL_SOURCE=str(root.parent / 'compiler-llvm-20261001/bc_source/Layers/W1/BaseApp'),
                   AGIRU_BC_SOURCE=str(root.parent / 'compiler-llvm-20261001/bc_source'),
                   AGIRU_BC_REVISION=inputs['bc_revision'],
                   AGIRU_SYSTEM_SYMBOLS=str(root.parent / 'compiler-llvm-20261001/system_symbols'))
started = time.monotonic()
with (root / 'artifacts/main-tests.log').open('w') as log:
    result = subprocess.run(['make', 'tc', 'test', 'JOBS=2'], cwd=main, env=environment, stdout=log, stderr=subprocess.STDOUT)
after = verify.digest(main)
receipt = {'command': ['make', 'tc', 'test', 'JOBS=2'], 'exit': result.returncode,
           'elapsed_seconds': time.monotonic() - started, 'source_before': before, 'source_after': after,
           'source_unchanged': before == after, 'frozen_inputs': inputs, 'complete_G1_proved': False}
(root / 'artifacts/main-tests.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert before == after
raise SystemExit(result.returncode)
