import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
libraries = source / 'build'
flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
         '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
         '-I' + str(source / 'include'), '-I' + str(source / 'test/gate')]
link = ['-L' + str(libraries), '-Wl,-rpath,' + str(libraries), '-lagiru_net', '-lagiru_al']
rows = []
for label in ('current', 'Remove', 'Clear'):
    units = [] if label == 'current' else [str(root / (label + 'PolicyMutant.cpp'))]
    target = root / 'artifacts' / ('iterator-' + label)
    command = [*flags, str(source / 'test/gate/DictionaryGate.cpp'), *units, *link, '-o', str(target)]
    compiled = subprocess.run(command, capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    executed = subprocess.run([str(target)], capture_output=True, text=True)
    assert executed.returncode == (0 if label == 'current' else 1), executed.stdout + executed.stderr
    if label != 'current':
        assert 'Dictionary changed during enumeration' in executed.stdout + executed.stderr
    row = {'label': label, 'command': command, 'exit': executed.returncode,
           'stdout': executed.stdout, 'stderr': executed.stderr}
    rows.append(row)
    print(json.dumps(row), flush=True)
(root / 'artifacts/iterator-controls.json').write_text(json.dumps(rows, indent=2) + '\n')
