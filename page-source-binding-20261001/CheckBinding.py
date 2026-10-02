import json
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
artifacts.mkdir(exist_ok=True)
label = sys.argv[1]
libraries = root.parent / 'page-table-field-20261001/source/build' if label == 'old' else source / 'build'
target = artifacts / ('binder-' + label)
command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
           '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
           '-I' + str(source / 'src/al'), '-I' + str(source / 'src/gen'),
           '-I' + str(source / 'include'), '-I' + str(source / 'test/gate'),
           str(source / 'test/gate/GenTableBindingGate.cpp'), '-L' + str(libraries),
           '-Wl,-rpath,' + str(libraries), '-lagiru_gen', '-lagiru_al', '-o', str(target)]
compiled = subprocess.run(command, capture_output=True, text=True)
assert compiled.returncode == 0, compiled.stderr
executed = subprocess.run([str(target)], capture_output=True, text=True)
(artifacts / (label + '-binding.json')).write_text(json.dumps({
    'command': command, 'exit': executed.returncode, 'stdout': executed.stdout,
    'stderr': executed.stderr}, indent=2) + '\n')
print(executed.stdout)
assert executed.returncode == (1 if label == 'old' else 0), executed.stderr
