import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import time

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
name = sys.argv[1] if len(sys.argv) > 1 else 'canonical-scope-generation'
output = source / 'build' / (name + '-generated')
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(source)
command = [str(source / 'build/agirutc'), '/home/cosmo/Git/BCApps/src',
           str(source / 'apps.json'), str(output)]
started = time.monotonic()
with (artifacts / (name + '.log')).open('w') as log:
    result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT,
                            env=dict(os.environ, AGIRU_SYSTEM_SYMBOLS=
                            '/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH'))
after = verify.digest(source)
names = {path.name for path in output.rglob('*') if path.is_file()}
receipt = {'command': command, 'exit': result.returncode,
           'elapsed_seconds': time.monotonic() - started, 'source_before': before,
           'source_after': after, 'source_unchanged': before == after,
           'generated_files': len(list(output.rglob('*.cpp'))) + len(list(output.rglob('*.h'))),
           'excluded_object_absent': not any(name.startswith('O365RoleCenterNotifications.') for name in names),
           'full_tree_or_G1_proved': False}
(artifacts / (name + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
assert receipt['source_unchanged'] and receipt['excluded_object_absent']
assert 'product exclusion licensing-and-microsoft-cloud:' in (
    artifacts / (name + '.log')).read_text()
print(json.dumps(receipt))
