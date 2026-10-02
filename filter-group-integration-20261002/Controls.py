import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys

task = Path(__file__).resolve().parent
source = task / 'source'
artifacts = task / 'artifacts'
prefix = sys.argv[1] + '-' if len(sys.argv) > 1 else ''
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identity = verify.digest(source)
flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '--rtlib=compiler-rt',
         '--unwindlib=libunwind', '-fuse-ld=lld-19', '-Wall', '-Wextra', '-Wpedantic',
         '-Werror', '-shared', '-fPIC', '-I' + str(source / 'include'),
         '-L' + str(source / 'build'), '-Wl,-rpath,' + str(source / 'build')]
rows = []
for name in ('GetterReset', 'UnboundedSetter', 'GroupBlind'):
    path = artifacts / (name + '.cpp')
    library = artifacts / (prefix + name + '.so')
    command = flags + [str(path), '-lagiru_rt', '-o', str(library)]
    compiled = subprocess.run(command, capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(source / 'build/gate_FilterGroupGate')],
        env=dict(os.environ, LD_PRELOAD=str(library)), capture_output=True, text=True)
    (artifacts / (prefix + name + '.log')).write_text(result.stdout + result.stderr)
    counts = re.search(r'FilterGroup: (\d+) check\(s\), (\d+) red', result.stdout)
    assert result.returncode == 1 and counts and int(counts[2]) > 0, result.stdout + result.stderr
    rows.append(dict(name=name, exit=result.returncode, checks=int(counts[1]), red=int(counts[2]),
        mutant_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
        mechanism='Interpose only the named operation in the matching tested LLVM image.'))
receipt = dict(source_sha256=identity, source_unchanged=identity == verify.digest(source),
               rows=rows, uses_database=False)
(artifacts / (prefix + 'controls.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert receipt['source_unchanged']
