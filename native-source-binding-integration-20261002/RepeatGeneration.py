import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import time

task = Path(__file__).resolve().parent
source = task / 'source'
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
old = json.loads((artifacts / 'generation.json').read_text())
inputs = old['frozen_inputs']
bc = task.parent / 'compiler-llvm-20261001/bc_source'
symbols = task.parent / 'compiler-llvm-20261001/system_symbols'

def generated():
    return {str(path.relative_to(source / 'apps')):
            (hashlib.sha256(path.read_bytes()).hexdigest(), path.stat().st_mtime_ns)
            for path in (source / 'apps').rglob('*') if path.is_file()}

before = verify.digest(source)
files_before = generated()
assert {path: identity for path, (identity, _) in files_before.items()} == old['generated_after_hashes']
environment = dict(os.environ, CCACHE_DIR=str(task / 'ccache'), AGIRU_BC_SOURCE=str(bc),
    AGIRU_BC_REVISION=inputs['bc_revision'], AGIRU_SYSTEM_SYMBOLS=str(symbols))
started = time.monotonic()
command = ['make', 'transpile', 'JOBS=2']
with (artifacts / 'repeat-generation.log').open('w') as log:
    result = subprocess.run(command, cwd=source, env=environment, stdout=log, stderr=subprocess.STDOUT)
files_after = generated()
receipt = dict(command=command, exit=result.returncode, elapsed_seconds=time.monotonic() - started,
    source_before=before, source_after=verify.digest(source),
    generated_files=len(files_after), byte_and_mtime_identical=files_before == files_after,
    changed=[path for path in files_before.keys() & files_after.keys() if files_before[path] != files_after[path]],
    added=sorted(files_after.keys() - files_before.keys()), removed=sorted(files_before.keys() - files_after.keys()),
    frozen_inputs_unchanged=verify.digest(bc) == inputs['bc_source_sha256'] and
                            verify.digest(symbols) == inputs['system_symbols_sha256'], G1_proved=False)
(artifacts / 'repeat-generation.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert result.returncode == 0 and receipt['byte_and_mtime_identical']
assert receipt['source_before'] == receipt['source_after'] and receipt['frozen_inputs_unchanged']
