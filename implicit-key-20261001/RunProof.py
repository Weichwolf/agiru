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
environment = os.environ.copy()
environment['CCACHE_DIR'] = '/tmp/agiru-native-20261001.WC8RiY/ccache'
environment['AGIRU_BC_SOURCE'] = '/home/cosmo/Git/BCApps/src'
environment['AGIRU_SYSTEM_SYMBOLS'] = '/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH'
actions = {
    'key-clang': ['make', 'gate', 'GATE=GenKeyGate', 'JOBS=2'],
    'tc': ['make', 'tc', 'JOBS=2'],
    'gates': ['make', 'gates', 'JOBS=2'],
    'test-clang': ['make', 'test', 'JOBS=2'],
    'test-clang-fixed': ['make', 'test', 'JOBS=2'],
    'test-clang-final': ['make', 'test', 'JOBS=2'],
    'test-clang-verified': ['make', 'test', 'JOBS=2'],
    'gcc': ['make', 'gcc', 'JOBS=2'],
    'test-gcc': ['make', 'test', 'B=build/gcc', 'CXX=g++-14', 'JOBS=2'],
    'fixture-clang': ['python3', 'test/toolchain.py', 'ImplicitPrimaryKeyGate', '-v'],
    'fixture-gcc': ['python3', 'test/toolchain.py', 'ImplicitPrimaryKeyGate', '-v'],
    'transpile': ['make', 'transpile', 'JOBS=2'],
    'transpile-final': ['make', 'transpile', 'JOBS=2'],
}
name = sys.argv[1]
if name not in actions:
    raise SystemExit('unknown proof action')
environment['B'] = 'build/gcc' if name.endswith('gcc') else 'build'
started = time.monotonic()
with (artifacts / f'{name}.log').open('w') as log:
    result = subprocess.run(actions[name], cwd=source, env=environment,
                            stdout=log, stderr=subprocess.STDOUT)
receipt = {'action': name, 'command': actions[name], 'exit': result.returncode,
           'elapsed_seconds': time.monotonic() - started}
(artifacts / f'{name}.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
raise SystemExit(result.returncode)
