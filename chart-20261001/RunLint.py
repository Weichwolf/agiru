from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parent
rows = []
for unit in ('src/gen/CodeunitWriter.cpp', 'src/gen/PageWriter.cpp', 'src/gen/Door.cpp', 'src/rt/PlatformTables.cpp'):
    label = Path(unit).stem.lower()
    result = subprocess.run([sys.executable, str(root / 'RunMake.py'), 'lint-' + label,
                             'lint-one', 'UNIT=' + unit])
    comparison = subprocess.run([sys.executable, str(root / 'CompareLint.py'), unit])
    rows.append({'unit': unit, 'lint_exit': result.returncode, 'comparison_exit': comparison.returncode})
    (root / 'lint-exits.json').write_text(json.dumps(rows, indent=2) + '\n')
    if result.returncode not in (0, 2) or comparison.returncode != 0:
        raise SystemExit('unexpected lint result: ' + unit)
