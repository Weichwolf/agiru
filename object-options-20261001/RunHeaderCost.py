from pathlib import Path
import json
import os
import subprocess
import time

root = Path(__file__).resolve().parent
source = root / 'source'
old = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
rows = []
for sample in range(2):
    for cxx in ('clang++-19', 'g++-14'):
        for label, image in [('predecessor', old), ('current', source)]:
            command = [cxx, '-std=c++23', '-Wall',
                       '-Wextra', '-Wpedantic', '-Werror', '-fsyntax-only',
                       '-I' + str(image / 'include'), str(root / 'HeaderCost.cpp')]
            log = root / 'artifacts' / f'header-cost-{sample}-{cxx}-{label}.log'
            started = time.monotonic()
            with log.open('w') as output:
                process = subprocess.Popen(command, stdout=output, stderr=output)
                _, status, usage = os.wait4(process.pid, 0)
                process.returncode = os.waitstatus_to_exitcode(status)
            rows.append({'sample': sample, 'compiler': cxx, 'image': label,
                         'command': command, 'exit': process.returncode,
                         'seconds': time.monotonic() - started, 'peak_kib': usage.ru_maxrss})
            if process.returncode != 0:
                raise SystemExit(log.read_text())
receipt = {'samples': rows, 'no_pch': True, 'single_header_only': True,
           'throughput_or_erp_performance_claim': False}
(root / 'artifacts/header-cost.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
