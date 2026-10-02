import hashlib
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
result = subprocess.run([str(root / 'context-probe'), str(root)], capture_output=True, text=True)
(root / 'context-probe.log').write_text(result.stdout + result.stderr)
rows = [json.loads(line) for line in result.stdout.splitlines()]
receipt = {'exit': result.returncode,
           'libxml2_version': subprocess.check_output(['pkg-config', '--modversion', 'libxml-2.0'], text=True).strip(),
           'source_sha256': hashlib.sha256((root / 'ContextProbe.cpp').read_bytes()).hexdigest(),
           'binary_sha256': hashlib.sha256((root / 'context-probe').read_bytes()).hexdigest(),
           'fixture_sha256': {name: hashlib.sha256((root / name).read_bytes()).hexdigest() for name in ('owned-entity.txt', 'owned-dtd.txt')},
           'uses_process_global_loader': False, 'production_reader_fixed': False, 'Ignore_policy_proved': False,
           'tested_cases': len(rows), 'rows': rows}
(root / 'context-probe.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key != 'rows'}))
assert result.returncode == 0 and len(rows) == 24
