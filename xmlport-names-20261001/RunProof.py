from pathlib import Path
import json
import hashlib
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
    'tc': ['make', 'tc', 'JOBS=2'],
    'gcc': ['make', 'gcc', 'JOBS=2'],
    'test-gcc': ['make', 'test', 'B=build/gcc', 'CXX=g++-14', 'JOBS=2'],
    'transpile': ['make', 'transpile', 'JOBS=2'],
    'transpile-repeat': ['make', 'transpile', 'JOBS=2'],
    'lint-main': ['make', 'lint-one', 'UNIT=src/tc/Main.cpp', 'JOBS=2'],
    'lint-codeunit': ['make', 'lint-one', 'UNIT=src/gen/CodeunitWriter.cpp', 'JOBS=2'],
    'lint-table': ['make', 'lint-one', 'UNIT=src/gen/TableWriter.cpp', 'JOBS=2'],
    'lint-body': ['make', 'lint-one', 'UNIT=src/gen/BodyWriter.cpp', 'JOBS=2'],
    'lint-page': ['make', 'lint-one', 'UNIT=src/gen/PageWriter.cpp', 'JOBS=2'],
}
name = sys.argv[1]
def image():
    return {str(path.relative_to(source / 'apps')):
            (hashlib.sha256(path.read_bytes()).hexdigest(), path.stat().st_mtime_ns)
            for path in (source / 'apps').rglob('*') if path.is_file()}

before = image() if name == 'transpile-repeat' else {}
started = time.monotonic()
with (artifacts / f'{name}.log').open('w') as log:
    result = subprocess.run(actions[name], cwd=source, env=environment,
                            stdout=log, stderr=subprocess.STDOUT)
if name.startswith('lint-'):
    for suffix in ('targeted.log', 'targeted-units.json'):
        (artifacts / f'{name}-{suffix}').write_bytes((source / 'build/lint' / suffix).read_bytes())
receipt = {'action': name, 'command': actions[name], 'exit': result.returncode,
           'elapsed_seconds': time.monotonic() - started}
(artifacts / f'{name}.json').write_text(json.dumps(receipt, indent=2) + '\n')
if name == 'transpile-repeat':
    after = image()
    retained = {'before_files': len(before), 'after_files': len(after),
                'missing': sorted(before.keys() - after.keys()),
                'added': sorted(after.keys() - before.keys()),
                'changed_bytes_or_mtime': sorted(path for path in before.keys() & after.keys()
                                               if before[path] != after[path])}
    (artifacts / 'repeat-output.json').write_text(json.dumps(retained, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
raise SystemExit(result.returncode)
