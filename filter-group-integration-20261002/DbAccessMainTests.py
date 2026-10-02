import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import time

task = Path(__file__).resolve().parent
root = task.parent.parent
artifacts = task / 'artifacts'
label = 'db-access-main-tests'
assert not (artifacts / (label + '.json')).exists()
spec = importlib.util.spec_from_file_location('verify_snapshot', root / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(root)
command = ['make', 'test', 'JOBS=2']
environment = dict(os.environ, CCACHE_DIR=str(root / 'build/native-record-context-integration-20261002/ccache'))
started = time.monotonic()
with (artifacts / (label + '.log')).open('w') as log:
    result = subprocess.run(command, cwd=root, env=environment, stdout=log, stderr=subprocess.STDOUT)
after = verify.digest(root)
output = (artifacts / (label + '.log')).read_text()
gates = {name: {'checks': int(checks), 'red': int(red)}
         for name, checks, red in re.findall(r'^([A-Za-z0-9_]+): (\d+) check\(s\), (\d+) red$', output, re.M)}
manifest = sorted(path.name for path in (root / 'test/gate').glob('*.cpp'))
own_output = (artifacts / 'db-access-tests.log').read_text()
own_gates = {name: {'checks': int(checks), 'red': int(red)}
             for name, checks, red in re.findall(r'^([A-Za-z0-9_]+): (\d+) check\(s\), (\d+) red$', own_output, re.M)}
receipt = dict(command=command, exit=result.returncode, elapsed_seconds=time.monotonic() - started,
               source_before=before, source_after=after, source_unchanged=before == after,
               cpp_manifest=manifest, cpp_results=gates, own_cpp_results_equal=gates == own_gates,
               cpp_complete=len(gates) == len(manifest), cpp_checks=sum(g['checks'] for g in gates.values()),
               complete_local_summary='test: 97 case(s), 0 red' in output,
               python_140_passed=bool(re.search(r'Ran 140 tests in .*\n\nOK\b', output)),
               database='agiru_gate', G1_proved=False)
(artifacts / (label + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key not in ('cpp_manifest', 'cpp_results')}))
assert before == after, 'source changed during the test run'
assert receipt['cpp_complete'] and receipt['own_cpp_results_equal']
assert all(g['red'] == 0 for g in gates.values())
assert receipt['complete_local_summary'] and receipt['python_140_passed']
raise SystemExit(result.returncode)
