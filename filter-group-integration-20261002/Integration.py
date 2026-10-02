import importlib.util
import json
import os
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
main = task.parent.parent
cache = task.parent / 'verify-cache-20261002/source'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
assert (task / 'artifacts/main-proof.json').exists()
latest = Path((cache / 'build/verify/latest').read_text().strip())
assert latest.is_relative_to(cache / 'build/verify')
previous = json.loads((latest / 'result.json').read_text())
assert previous['status'] in ('failed', 'passed')
assert verify.digest(Path(previous['source'])) == previous['source_sha256']
before = verify.digest(main)
old_cache = verify.digest(cache)
verify.sync_source(main, cache)
assert before == verify.digest(main) == verify.digest(cache)
inputs = json.loads((task.parent / 'compiler-llvm-20261001/artifacts/inputs.json').read_text())
bc = task.parent / 'compiler-llvm-20261001/bc_source'
symbols = task.parent / 'compiler-llvm-20261001/system_symbols'
assert verify.digest(bc) == inputs['bc_source_sha256']
assert verify.digest(symbols) == inputs['system_symbols_sha256']
receipt = {'main_source_sha256': before, 'copied_source_sha256': before,
    'prior_cache_source_sha256': old_cache, 'prior_snapshot': str(latest),
    'prior_snapshot_preserved': verify.digest(Path(previous['source'])) == previous['source_sha256'],
    'frozen_inputs': inputs}
(task / 'artifacts/integration-origin.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
environment = dict(os.environ,
    AGIRU_BC_SOURCE=str(bc), AGIRU_BC_REVISION=inputs['bc_revision'], AGIRU_SYSTEM_SYMBOLS=str(symbols),
    CCACHE_DIR=str(task.parent / 'native-record-context-integration-20261002/ccache'))
result = subprocess.run(['make', 'verify', 'JOBS=6'], cwd=cache, env=environment)
raise SystemExit(result.returncode)
