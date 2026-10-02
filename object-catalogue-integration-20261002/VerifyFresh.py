import importlib.util
import json
import os
from pathlib import Path
import subprocess
import time

root = Path(__file__).resolve().parent
main = root.parent.parent
clone = root.parent / 'verify-cache-20261002/source'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
previous = Path((clone / 'build/verify/latest').read_text().strip())
assert json.loads((previous / 'result.json').read_text())['status'] in ('passed', 'failed')
assert (clone / '.git').is_dir() and clone != main
expected = verify.digest(main)
verify.sync_source(main, clone)
assert verify.digest(main) == verify.digest(clone) == expected
head = subprocess.check_output(['git', '-C', str(main), 'rev-parse', 'HEAD'], text=True).strip()
assert subprocess.check_output(['git', '-C', str(clone), 'rev-parse', 'HEAD'], text=True).strip() == head
inputs = json.loads((root.parent / 'compiler-llvm-20261001/artifacts/inputs.json').read_text())
environment = dict(os.environ, CCACHE_DIR=str(root.parent / 'dictionary-integration-20261002/ccache'),
                   AGIRU_AL_SOURCE=str(root.parent / 'compiler-llvm-20261001/bc_source/Layers/W1/BaseApp'),
                   AGIRU_BC_SOURCE=str(root.parent / 'compiler-llvm-20261001/bc_source'),
                   AGIRU_BC_REVISION=inputs['bc_revision'],
                   AGIRU_SYSTEM_SYMBOLS=str(root.parent / 'compiler-llvm-20261001/system_symbols'))
before_stats = subprocess.run(['ccache', '--print-stats'], env=environment, capture_output=True, text=True)
assert before_stats.returncode == 0, before_stats.stderr
(root / 'artifacts/verify-cache-before.txt').write_text(before_stats.stdout)
identity = {'main_source_sha256': expected, 'copied_source_sha256': verify.digest(clone),
            'origin': str(main), 'source': str(clone), 'head': head, 'frozen_inputs': inputs,
            'previous_terminal_snapshot': str(previous)}
(root / 'artifacts/verify-source-identity.json').write_text(json.dumps(identity, indent=2) + '\n')
print(json.dumps(identity), flush=True)
started = time.monotonic()
with (root / 'artifacts/verify-launch.log').open('w') as log:
    result = subprocess.run(['make', 'verify', 'JOBS=6'], cwd=clone, env=environment, stdout=log, stderr=subprocess.STDOUT)
after_stats = subprocess.run(['ccache', '--print-stats'], env=environment, capture_output=True, text=True)
assert after_stats.returncode == 0, after_stats.stderr
(root / 'artifacts/verify-cache-after.txt').write_text(after_stats.stdout)
snapshot = Path((clone / 'build/verify/latest').read_text().strip())
assert snapshot != previous
metadata = json.loads((snapshot / 'result.json').read_text())
receipt = {'command': ['make', 'verify', 'JOBS=6'], 'exit': result.returncode, 'snapshot': str(snapshot),
           'elapsed_seconds': time.monotonic() - started,
           'source_before': expected, 'source_after': verify.digest(clone),
           'source_unchanged': expected == verify.digest(clone), 'snapshot_result': metadata}
(root / 'artifacts/verification.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
assert receipt['source_unchanged'] and metadata['status'] in ('passed', 'failed')
raise SystemExit(result.returncode)
