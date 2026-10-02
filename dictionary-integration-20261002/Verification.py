import importlib.util
import json
import os
from pathlib import Path
import subprocess
import time

root = Path(__file__).resolve().parent
repository = root.parent.parent
proof_root = root.parent / 'verify-cache-20261002'
source = proof_root / 'source'
artifacts = proof_root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', repository / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
if source.exists():
    raise RuntimeError('refusing to overwrite a source image or restart an existing run')
artifacts.mkdir(parents=True, exist_ok=True)
identity = verify.digest(repository)
subprocess.run(['git', 'clone', '--shared', '--no-checkout', str(repository), str(source)], check=True)
verify.copy_source(repository, source)
assert verify.digest(source) == verify.digest(repository) == identity
inputs = json.loads((root.parent / 'compiler-llvm-20261001/artifacts/inputs.json').read_text())
compiler = root.parent / 'compiler-llvm-20261001'
environment = dict(os.environ, CCACHE_DIR=str(root / 'ccache'),
                   AGIRU_BC_SOURCE=str(compiler / 'bc_source'),
                   AGIRU_BC_REVISION=inputs['bc_revision'],
                   AGIRU_AL_SOURCE=str(compiler / 'bc_source/Layers/W1/BaseApp'),
                   AGIRU_SYSTEM_SYMBOLS=str(compiler / 'system_symbols'))
(artifacts / 'source-identity.json').write_text(json.dumps({
    'main_source_sha256': identity, 'copied_source_sha256': identity,
    'origin': str(repository), 'source': str(source), 'frozen_inputs': inputs}, indent=2) + '\n')
started = time.monotonic()
with (artifacts / 'verification.log').open('w') as log:
    result = subprocess.run(['make', 'verify', 'JOBS=6'], cwd=source, env=environment,
                            stdout=log, stderr=subprocess.STDOUT)
receipt = {'exit': result.returncode, 'elapsed_seconds': time.monotonic() - started,
           'source_before': identity, 'source_after': verify.digest(source),
           'source_unchanged': identity == verify.digest(source)}
(artifacts / 'verification.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert receipt['source_unchanged']
raise SystemExit(result.returncode)
