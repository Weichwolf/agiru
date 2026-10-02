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
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
manifest_spec = importlib.util.spec_from_file_location('ut_manifest', source / 'scripts/ut_manifest.py')
manifest = importlib.util.module_from_spec(manifest_spec)
manifest_spec.loader.exec_module(manifest)
bc = root.parent / 'compiler-llvm-20261001/bc_source'


def outputs():
    return {str(path.relative_to(source / 'apps')): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in (source / 'apps').rglob('*') if path.is_file()}


before = verify.digest(source)
raw_before = manifest.scan(bc / 'Layers/W1/Tests')
previous = outputs()
environment = dict(os.environ, CCACHE_DIR=str(root / 'ccache'), AGIRU_BC_SOURCE=str(bc),
                   AGIRU_SYSTEM_SYMBOLS=str(root.parent / 'compiler-llvm-20261001/system_symbols'))
command = ['make', 'transpile', 'JOBS=6']
started = time.monotonic()
with (artifacts / 'generation.log').open('w') as log:
    result = subprocess.run(command, cwd=source, env=environment, stdout=log, stderr=subprocess.STDOUT)
after = verify.digest(source)
assert raw_before == manifest.scan(bc / 'Layers/W1/Tests')
current = outputs()
assert previous.keys() == current.keys()
assert not any('O365RoleCenterNotifications.' in path for path in current)
assert 'tests/core/codeunit/O365TrialBalance.cpp' in current
slice_sources = [line.strip() for line in (source / 'test/slice').read_text().splitlines()
                 if line.strip() and not line.lstrip().startswith('#')]
missing = sorted(set(slice_sources) - current.keys())
receipt = {'command': command, 'exit': result.returncode, 'source_before': before, 'source_after': after,
           'elapsed_seconds': time.monotonic() - started, 'generated_files': len(current),
           'changed_outputs': sorted(path for path in previous if previous[path] != current[path]),
           'deleted_outputs': [], 'ut_codeunits': len(raw_before),
           'ut_methods': sum(len(item['methods']) for item in raw_before),
           'ut_identity_gains': 0, 'ut_identity_losses': 0, 'compiled_slice_sources': len(slice_sources),
           'missing_slice_sources': missing, 'full_tree_or_G1_proved': False}
(artifacts / 'generation.json').write_text(json.dumps(receipt, indent=2) + '\n')
(artifacts / 'raw-manifest.json').write_text(json.dumps(raw_before, indent=2) + '\n')
print(json.dumps(receipt))
assert result.returncode == 0 and not missing
