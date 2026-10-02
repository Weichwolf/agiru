import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import time

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
compiler = root.parent / 'compiler-llvm-20261001'
bc = compiler / 'bc_source'
inputs = json.loads((compiler / 'artifacts/inputs.json').read_text())
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
manifest_spec = importlib.util.spec_from_file_location('ut_manifest', source / 'scripts/ut_manifest.py')
manifest = importlib.util.module_from_spec(manifest_spec)
manifest_spec.loader.exec_module(manifest)


def outputs():
    return {path.relative_to(source / 'apps').as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in (source / 'apps').rglob('*') if path.is_file()}


before = verify.digest(source)
old = outputs()
raw = manifest.scan(bc / 'Layers/W1/Tests')
assert len(raw) == 80 and sum(len(row['methods']) for row in raw) == 2310
(artifacts / 'outputs-before.json').write_text(json.dumps(old, indent=2) + '\n')
environment = dict(os.environ, CCACHE_DIR=str(root / 'ccache'),
                   AGIRU_BC_SOURCE=str(bc), AGIRU_BC_REVISION=inputs['bc_revision'],
                   AGIRU_AL_SOURCE=str(bc / 'Layers/W1/BaseApp'),
                   AGIRU_SYSTEM_SYMBOLS=str(compiler / 'system_symbols'))
started = time.monotonic()
with (artifacts / 'generation.log').open('w') as log:
    result = subprocess.run(['make', 'transpile', 'JOBS=6'], cwd=source, env=environment,
                            stdout=log, stderr=subprocess.STDOUT)
current = outputs()
slice_sources = [line.strip() for line in (source / 'test/slice').read_text().splitlines()
                 if line.strip() and not line.lstrip().startswith('#')]
missing = sorted(set(slice_sources) - current.keys())
assert raw == manifest.scan(bc / 'Layers/W1/Tests')
receipt = {'exit': result.returncode, 'elapsed_seconds': time.monotonic() - started,
           'source_before': before, 'source_after': verify.digest(source),
           'generated_before': len(old), 'generated_after': len(current),
           'added': sorted(current.keys() - old.keys()), 'removed': sorted(old.keys() - current.keys()),
           'changed': sorted(path for path in old.keys() & current.keys() if old[path] != current[path]),
           'active_slice_sources': len(slice_sources), 'missing_slice_sources': missing,
           'ut_codeunits': len(raw), 'ut_methods': 2310, 'ut_identity_gains': 0, 'ut_identity_losses': 0,
           'full_tree_or_G1_proved': False, 'frozen_inputs': inputs}
(artifacts / 'generation.json').write_text(json.dumps(receipt, indent=2) + '\n')
(artifacts / 'outputs-after.json').write_text(json.dumps(current, indent=2) + '\n')
(artifacts / 'raw-manifest.json').write_text(json.dumps(raw, indent=2) + '\n')
print(json.dumps(receipt))
assert result.returncode == 0 and not missing
