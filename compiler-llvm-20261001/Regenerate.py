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
bc = Path('/home/cosmo/Git/BCApps/src')
before = verify.digest(source)
raw_before = manifest.scan(bc / 'Layers/W1/Tests')
environment = dict(os.environ, CCACHE_DIR=str(root / 'ccache'), AGIRU_BC_SOURCE=str(bc),
                   AGIRU_SYSTEM_SYMBOLS='/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH')
command = ['make', 'transpile', 'JOBS=6']
started = time.monotonic()
with (artifacts / 'generation.log').open('w') as log:
    result = subprocess.run(command, cwd=source, env=environment, stdout=log, stderr=subprocess.STDOUT)
after = verify.digest(source)
assert raw_before == manifest.scan(bc / 'Layers/W1/Tests')
paths = {str(path.relative_to(source / 'apps')) for path in (source / 'apps').rglob('*') if path.is_file()}
assert not any('O365RoleCenterNotifications.' in path for path in paths)
assert 'tests/core/codeunit/O365TrialBalance.cpp' in paths
slice_sources = [line.strip() for line in (source / 'test/slice').read_text().splitlines()
                 if line.strip() and not line.lstrip().startswith('#')]
missing = sorted(set(slice_sources) - paths)
receipt = {'command': command, 'exit': result.returncode, 'source_before': before, 'source_after': after,
           'elapsed_seconds': time.monotonic() - started, 'generated_files': len(paths),
           'ut_codeunits': len(raw_before), 'ut_methods': sum(len(item['methods']) for item in raw_before),
           'ut_identity_gains': 0, 'ut_identity_losses': 0, 'compiled_slice_sources': len(slice_sources),
           'explicit_product_removed_slice_sources': ['tests/core/codeunit/O365RoleCenterNotifications.cpp'],
           'missing_slice_sources': missing, 'full_tree_or_G1_proved': False}
(artifacts / 'generation.json').write_text(json.dumps(receipt, indent=2) + '\n')
(artifacts / 'raw-manifest.json').write_text(json.dumps(raw_before, indent=2) + '\n')
print(json.dumps(receipt))
assert result.returncode == 0 and not missing
