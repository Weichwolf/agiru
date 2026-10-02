from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parent
result = subprocess.run([sys.executable, str(root / 'RunMake.py'), 'lint-storage',
                         'lint-one', 'UNIT=src/rt/Storage.cpp'])
if result.returncode != 2:
    raise SystemExit('unexpected Storage analysis status')
raise SystemExit(subprocess.run([sys.executable, str(root / 'CompareLint.py'), 'src/rt/Storage.cpp']).returncode)
