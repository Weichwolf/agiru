import json
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root.parent.parent
label = sys.argv[1]
cases = ['SnapshotGate.test_deleted_tracked_caches_do_not_abort_freezing'] if label == 'old' else ['SnapshotGate']
result = subprocess.run([sys.executable, 'test/toolchain.py', *cases], cwd=source,
                        capture_output=True, text=True)
receipt = {'label': label, 'exit': result.returncode, 'stdout': result.stdout, 'stderr': result.stderr}
(root / 'artifacts' / ('snapshot-cache-' + label + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(result.stdout + result.stderr)
assert result.returncode == (1 if label == 'old' else 0)
