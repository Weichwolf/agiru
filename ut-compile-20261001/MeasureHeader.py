from pathlib import Path
import json
import os
import statistics
import subprocess
import time

root = Path(__file__).resolve().parent
current = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
rows = []
for mode, source in (('before', origin), ('after', current)):
    measurements = []
    for iteration in range(3):
        metrics = root / 'artifacts' / f'header-cost-{mode}-{iteration}.json'
        command = ['clang++-19', '-std=c++23', '-Wall', '-Wextra',
                   '-Wpedantic', '-Werror', '-fsyntax-only', '-I' + str(source / 'include'),
                   '-I' + str(source / 'test/gate'), str(root / 'FieldBoundary.cpp')]
        started = time.monotonic()
        process = subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        _, status, usage = os.wait4(process.pid, 0)
        process.returncode = os.waitstatus_to_exitcode(status)
        error = process.stderr.read().decode()
        process.stderr.close()
        if process.returncode != 0:
            raise RuntimeError(error)
        measured = {'elapsed_seconds': time.monotonic() - started,
                    'maximum_rss_kib': usage.ru_maxrss}
        metrics.write_text(json.dumps(measured) + '\n')
        measurements.append(measured)
    rows.append({'mode': mode, 'source': str(source), 'measurements': measurements,
                 'median_elapsed_seconds': statistics.median(row['elapsed_seconds'] for row in measurements),
                 'median_maximum_rss_kib': statistics.median(row['maximum_rss_kib'] for row in measurements)})
receipt = {'compiler': 'clang++-19', 'sample': 'FieldBoundary.cpp header-only compilation',
           'measurements': rows, 'full_build_cost_proved': False,
           'concurrent_local_build_active': True, 'erp_performance_claim': False}
(root / 'artifacts/header-cost.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
