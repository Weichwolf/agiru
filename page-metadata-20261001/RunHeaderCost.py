from pathlib import Path
import json
import os
import subprocess
import sys
import time

root = Path(__file__).resolve().parent
image = root / 'source'
phase = sys.argv[1]
if phase not in ('before', 'after'):
    raise SystemExit('expected before|after')
rows = []
for sample in range(2):
    for cxx in ('clang++-19', 'g++-14'):
        command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                   '-fsyntax-only', '-I' + str(image / 'include'), str(root / 'HeaderCost.cpp')]
        log = root / 'artifacts' / f'header-cost-{phase}-{sample}-{cxx}.log'
        started = time.monotonic()
        with log.open('w') as output:
            process = subprocess.Popen(command, stdout=output, stderr=output)
            _, status, usage = os.wait4(process.pid, 0)
            process.returncode = os.waitstatus_to_exitcode(status)
        rows.append({'sample': sample, 'compiler': cxx, 'phase': phase, 'exit': process.returncode,
                     'seconds': time.monotonic() - started, 'peak_kib': usage.ru_maxrss})
        if process.returncode != 0:
            raise SystemExit(log.read_text())
receipt = {'samples': rows, 'no_pch': True, 'single_header_only': True,
           'erp_performance_claim': False}
(root / 'artifacts' / f'header-cost-{phase}.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
