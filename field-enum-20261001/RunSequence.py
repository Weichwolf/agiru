from pathlib import Path
import json
import os
import subprocess
import sys

root = Path(__file__).resolve().parent
steps = [
    ('test-gcc', ['test', 'B=build/gcc', 'CXX=g++-14'], 2),
    ('lint-bodywriter', ['lint-one', 'UNIT=src/gen/BodyWriter.cpp'], 2),
    ('lint-codeunitwriter', ['lint-one', 'UNIT=src/gen/CodeunitWriter.cpp'], 2),
    ('transpile', ['transpile'], 0),
    ('transpile-repeat', ['transpile'], 0),
    ('gap', ['gap', 'SOURCE=1'], 2),
    ('ut', ['ut'], 2),
]
rows = []
for name, arguments, expected in steps:
    environment = dict(os.environ, AGIRU_PROOF_JOBS='6' if name == 'ut' else '2')
    result = subprocess.run([sys.executable, str(root / 'RunMake.py'), name, *arguments],
                            env=environment)
    rows.append({'step': name, 'exit': result.returncode, 'expected': expected})
    (root / 'sequence-exits.json').write_text(json.dumps(rows, indent=2) + '\n')
    if result.returncode != expected:
        raise SystemExit('unexpected terminal status: ' + name)
    if name.startswith('lint-'):
        unit = 'src/gen/' + ('BodyWriter.cpp' if name == 'lint-bodywriter' else 'CodeunitWriter.cpp')
        compared = subprocess.run([sys.executable, str(root / 'CompareLint.py'), unit])
        if compared.returncode != 0:
            raise SystemExit('new lint finding: ' + unit)
print(json.dumps({'steps': len(rows), 'terminal': True}), flush=True)
