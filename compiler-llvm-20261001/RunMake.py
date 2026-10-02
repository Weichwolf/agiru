import hashlib
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
artifacts.mkdir(exist_ok=True)
name, *targets = sys.argv[1:]
if not targets or 'gcc' in targets:
    raise SystemExit('name and current Make targets required')
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
environment = dict(os.environ, CCACHE_DIR=str(root / 'ccache'),
                   AGIRU_BC_SOURCE=str(root / 'bc_source'),
                   AGIRU_SYSTEM_SYMBOLS=str(root / 'system_symbols'))
jobs = os.environ.get('AGIRU_PROOF_JOBS', '6')
assert jobs in ('2', '6')
command = ['make', *targets, 'JOBS=' + jobs]
before = verify.digest(source)
cache_before = subprocess.run(['ccache', '--show-stats', '--format=json'], env=environment,
                              text=True, capture_output=True)
started = time.monotonic()
with (artifacts / (name + '.log')).open('w') as log:
    result = subprocess.run(command, cwd=source, env=environment,
                            stdout=log, stderr=subprocess.STDOUT)
after = verify.digest(source)
cache_after = subprocess.run(['ccache', '--show-stats', '--format=json'], env=environment,
                             text=True, capture_output=True)
receipt = {'command': command, 'exit': result.returncode,
           'elapsed_seconds': time.monotonic() - started,
           'source_before': before, 'source_after': after, 'source_unchanged': before == after,
           'cache_before': cache_before.stdout, 'cache_after': cache_after.stdout}
(artifacts / (name + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
if before != after:
    raise SystemExit('source changed during verification')
raise SystemExit(result.returncode)
