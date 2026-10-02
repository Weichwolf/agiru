import hashlib
import json
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
label = sys.argv[1]
receipt_label = sys.argv[2] if len(sys.argv) > 2 else label
library = source / 'build/libagiru_gen.so'
identity = hashlib.sha256(library.read_bytes()).hexdigest()
target = root / 'artifacts' / ('receiver-' + receipt_label)
old_body = root.parent / 'page-source-binding-20261001/source/src/gen/BodyWriter.cpp'
units = [str(old_body)] if label == 'old' else []
command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
           '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
           '-I' + str(source / 'src/al'), '-I' + str(source / 'src/gen'),
           '-I' + str(source / 'include'), '-I' + str(source / 'test/gate'),
           str(source / 'test/gate/GenTableBindingGate.cpp'), *units, '-L' + str(library.parent),
           '-Wl,-rpath,' + str(library.parent), '-lagiru_gen', '-lagiru_al', '-o', str(target)]
compiled = subprocess.run(command, capture_output=True, text=True)
assert compiled.returncode == 0, compiled.stderr
executed = subprocess.run([str(target)], capture_output=True, text=True)
assert identity == hashlib.sha256(library.read_bytes()).hexdigest()
receipt = {'command': command, 'library_sha256': identity, 'exit': executed.returncode,
           'old_body_sha256': hashlib.sha256(old_body.read_bytes()).hexdigest() if label == 'old' else None,
           'stdout': executed.stdout, 'stderr': executed.stderr}
(root / 'artifacts' / ('receiver-' + receipt_label + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(executed.stdout)
assert executed.returncode == (1 if label == 'old' else 0), executed.stderr
