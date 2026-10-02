import json
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
artifacts.mkdir(exist_ok=True)
label = sys.argv[1]
source = root / 'source' if label == 'new' else root.parent / 'page-source-binding-20261001/source'
libraries = root.parent / 'page-source-binding-20261001/source/build'
flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
         '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
         '-I' + str(source / 'include'), '-I' + str(source / 'test/gate')]
programs = [('baseline', root / 'Baseline.cpp')]
if label == 'new':
    programs.append(('full', source / 'test/gate/DictionaryGate.cpp'))
receipts = []
for name, program in programs:
    target = artifacts / ('dictionary-' + label + '-' + name)
    units = [str(source / 'src/net/Generic.cpp')] if label == 'new' else []
    command = [*flags, str(program), *units, '-L' + str(libraries),
               '-Wl,-rpath,' + str(libraries), '-lagiru_net', '-lagiru_al', '-o', str(target)]
    compiled = subprocess.run(command, capture_output=True, text=True)
    if compiled.returncode != 0:
        (artifacts / ('compile-' + label + '-' + name + '.log')).write_text(compiled.stderr)
    assert compiled.returncode == 0, compiled.stderr
    executed = subprocess.run([str(target)], capture_output=True, text=True)
    receipts.append({'name': name, 'command': command, 'exit': executed.returncode,
                     'stdout': executed.stdout, 'stderr': executed.stderr})
    print(executed.stdout)
    assert executed.returncode == (1 if label == 'old' else 0), executed.stderr
(artifacts / ('dictionary-' + label + '.json')).write_text(json.dumps(receipts, indent=2) + '\n')
