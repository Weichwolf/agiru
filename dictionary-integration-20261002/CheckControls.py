import hashlib
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
repository = root.parent.parent
source = root / 'source'
libraries = source / 'build'
flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
         '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
         '-DAGIRU_SOURCE_DIR="' + str(source) + '"',
         '-I' + str(source / 'src/al'), '-I' + str(source / 'src/gen'),
         '-I' + str(source / 'include'), '-I' + str(source / 'test/gate')]
rows = []
for label in ('current', 'old-receiver', 'old-includes'):
    previous = None if label == 'current' else repository / ('src/gen/BodyWriter.cpp' if label == 'old-receiver' else 'src/gen/Door.cpp')
    units = [] if previous is None else [str(previous)]
    target = root / 'artifacts' / label
    command = [*flags, str(source / 'test/gate/GenReceiverGate.cpp'), *units,
               '-L' + str(libraries), '-Wl,-rpath,' + str(libraries),
               '-lagiru_gen', '-lagiru_al', '-o', str(target)]
    compiled = subprocess.run(command, capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    executed = subprocess.run([str(target)], capture_output=True, text=True)
    assert executed.returncode == (0 if previous is None else 1), executed.stdout + executed.stderr
    expected = {'current': '10 check(s), 0 red', 'old-receiver': '10 check(s), 3 red',
                'old-includes': '10 check(s), 1 red'}[label]
    assert expected in executed.stdout, executed.stdout + executed.stderr
    row = {'label': label, 'command': command, 'exit': executed.returncode,
           'before_unit': str(previous) if previous else None,
           'before_unit_sha256': hashlib.sha256(previous.read_bytes()).hexdigest() if previous else None,
           'stdout': executed.stdout, 'stderr': executed.stderr}
    rows.append(row)
    print(json.dumps(row), flush=True)
(root / 'artifacts/controls.json').write_text(json.dumps(rows, indent=2) + '\n')
