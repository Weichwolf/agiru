import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import time

task = Path(__file__).resolve().parent
source = task / 'source'
artifacts = task / 'artifacts'
label, *targets = sys.argv[1:]
assert targets and not (artifacts / (label + '.json')).exists()
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
environment = dict(os.environ, CCACHE_DIR=str(task.parent / 'native-record-context-integration-20261002/ccache'))
command = ['make', *targets, 'JOBS=2']
before = verify.digest(source)
started = time.monotonic()
with (artifacts / (label + '.log')).open('w') as log:
    result = subprocess.run(command, cwd=source, env=environment, stdout=log, stderr=subprocess.STDOUT)
after = verify.digest(source)
receipt = dict(command=command, exit=result.returncode, elapsed_seconds=time.monotonic() - started,
               source_before=before, source_after=after, source_unchanged=before == after)
(artifacts / (label + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert before == after, 'source changed during the build'
raise SystemExit(result.returncode)
