from pathlib import Path
import json
import os
import subprocess
import sys
import time

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
artifacts.mkdir(exist_ok=True)
environment = dict(os.environ, CCACHE_DIR='/tmp/agiru-native-20261001.WC8RiY/ccache',
                   AGIRU_BC_SOURCE='/home/cosmo/Git/BCApps/src',
                   AGIRU_SYSTEM_SYMBOLS='/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH')
actions = {
    'test-clang': ['make', 'test', 'JOBS=2'],
    'gcc': ['make', 'gcc', 'JOBS=2'],
    'test-gcc': ['make', 'test', 'B=build/gcc', 'CXX=g++-14', 'JOBS=2'],
    'transpile': ['make', 'transpile', 'JOBS=2'],
    'lint': ['make', 'lint-one', 'UNIT=src/tc/Main.cpp', 'JOBS=2'],
}
name = sys.argv[1]
started = time.monotonic()
with (artifacts / f'{name}.log').open('w') as log:
    result = subprocess.run(actions[name], cwd=source, env=environment,
                            stdout=log, stderr=subprocess.STDOUT)
receipt = {'action': name, 'command': actions[name], 'exit': result.returncode,
           'elapsed_seconds': time.monotonic() - started}
(artifacts / f'{name}.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
raise SystemExit(result.returncode)
