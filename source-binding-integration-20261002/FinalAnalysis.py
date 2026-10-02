import concurrent.futures
import importlib.util
import json
from pathlib import Path
import re
import subprocess

task = Path(__file__).resolve().parent
source = task / 'source'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identity = verify.digest(source)
prior = json.loads((task / 'artifacts/final-lint/targeted-lint.json').read_text())
units = ['src/gen/TableWriter.cpp', 'src/gen/BodyWriter.cpp', 'src/gen/CodeunitWriter.cpp',
         'src/tc/Main.cpp', 'test/gate/GenSourceBindingGate.cpp']

def analyse(unit):
    command = ['clang-tidy-19', '-p', str(source), '--quiet',
        '--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang',
        '--extra-arg=max-nodes=50000',
        '--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang',
        '--extra-arg=optin.performance.Padding:AllowedPad=64', str(source / unit)]
    result = subprocess.run(command, cwd=source, capture_output=True, text=True)
    output = result.stdout + result.stderr
    (task / 'artifacts' / ('final-analysis-' + Path(unit).stem + '.log')).write_text(output)
    findings = {re.sub(r':\d+:\d+:', ':LOC:', line.replace(str(source), '<source>'))
        for line in output.splitlines() if ': error:' in line or ': warning:' in line}
    baseline = next(row for row in prior['rows'] if row['unit'] == unit)
    old = set(baseline['results'].get('before', {}).get('findings', []))
    row = dict(unit=unit, exit=result.returncode, findings=sorted(findings),
        before_findings=sorted(old), new_findings=sorted(findings - old),
        removed_findings=sorted(old - findings))
    print(json.dumps({'unit': unit, 'before': len(old), 'after': len(findings),
        'new': row['new_findings']}), flush=True)
    return row

with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
    rows = list(pool.map(analyse, units))
receipt = dict(source_sha256=identity, source_unchanged=verify.digest(source) == identity,
    rows=rows, full_lint_passed=False, baselines_raised=False)
(task / 'artifacts/final-analysis.json').write_text(json.dumps(receipt, indent=2) + '\n')
assert receipt['source_unchanged'] and not any(row['new_findings'] for row in rows)
