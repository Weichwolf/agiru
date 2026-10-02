from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parent
rows = []
for unit in ('CodeunitWriter', 'TableWriter'):
    label = unit.lower()
    result = subprocess.run([sys.executable, str(root / 'RunMake.py'), 'lint-' + label,
                             'lint-one', 'UNIT=src/gen/' + unit + '.cpp'])
    comparison = subprocess.run([sys.executable, str(root / 'CompareLint.py'),
                                 'src/gen/' + unit + '.cpp'])
    rows.append({'unit': unit, 'lint_exit': result.returncode, 'comparison_exit': comparison.returncode})
    (root / 'lint-exits.json').write_text(json.dumps(rows, indent=2) + '\n')
    if result.returncode != 2 or comparison.returncode != 0:
        raise SystemExit('unexpected lint result: ' + unit)
