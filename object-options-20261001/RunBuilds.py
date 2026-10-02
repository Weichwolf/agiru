from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parent
steps = [
    ('test', ['test'], 2),
    ('tc', ['tc'], 0),
    ('gcc', ['gcc'], 0),
    ('test-gcc', ['test', 'B=build/gcc', 'CXX=g++-14'], 2),
    ('transpile', ['transpile'], 0),
    ('transpile-repeat', ['transpile'], 0),
]
rows = []
for name, arguments, expected in steps:
    result = subprocess.run([sys.executable, str(root / 'RunMake.py'), name, *arguments])
    rows.append({'step': name, 'exit': result.returncode, 'expected': expected})
    (root / 'build-exits.json').write_text(json.dumps(rows, indent=2) + '\n')
    if result.returncode != expected:
        raise SystemExit('unexpected terminal status: ' + name)
print(json.dumps({'steps': len(rows), 'terminal': True}), flush=True)
