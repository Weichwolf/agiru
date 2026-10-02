from pathlib import Path
import json
import shutil
import subprocess
import sys

root = Path(__file__).resolve().parent
archive = root / 'artifacts/before-gate-owner-fix'
archive.mkdir(exist_ok=False)
for name in ('test.log', 'test.json', 'test-gcc.log', 'test-gcc.json'):
    shutil.copy2(root / 'artifacts' / name, archive / name)
rows = []
for name, arguments, expected in [('test', ['test'], 2), ('tc', ['tc'], 0),
                                  ('gcc', ['gcc'], 0),
                                  ('test-gcc', ['test', 'B=build/gcc', 'CXX=g++-14'], 2)]:
    result = subprocess.run([sys.executable, str(root / 'RunMake.py'), name, *arguments])
    rows.append({'step': name, 'exit': result.returncode, 'expected': expected})
    (root / 'final-build-exits.json').write_text(json.dumps(rows, indent=2) + '\n')
    if result.returncode != expected:
        raise SystemExit('unexpected terminal status: ' + name)
print(json.dumps({'steps': len(rows), 'terminal': True}), flush=True)
