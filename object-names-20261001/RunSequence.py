from pathlib import Path
import json
import os
import subprocess
import sys

root = Path(__file__).resolve().parent
results = []
for name in ('test-clang', 'lint-main', 'lint-names', 'lint-table', 'lint-body',
             'gcc', 'test-gcc'):
    status = subprocess.call([sys.executable, str(root / 'RunProof.py'), name])
    results.append({'action': name, 'exit': status})
for compiler, build in (('clang', 'build'), ('gcc', 'build/gcc')):
    for mode in ('current', 'old-generator'):
        environment = dict(os.environ, B=build)
        with (root / 'artifacts' / f'controls-{mode}-{compiler}.log').open('w') as log:
            status = subprocess.call([sys.executable, str(root / 'RunControls.py'), mode],
                                     cwd=root / 'source', env=environment,
                                     stdout=log, stderr=subprocess.STDOUT)
        results.append({'action': f'controls-{mode}-{compiler}', 'exit': status})
        print(json.dumps(results[-1]), flush=True)
(root / 'proof-exits.json').write_text(json.dumps(results, indent=2) + '\n')
