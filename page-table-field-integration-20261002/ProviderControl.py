import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
source = task / 'source'
artifacts = task / 'artifacts'
binary = artifacts / 'provider-mutant'
libraries = source / 'build'
command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
           '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
           '-I' + str(source / 'include'), '-I' + str(source / 'test/gate'),
           str(source / 'test/gate/PageTableFieldGate.cpp'), str(task / 'ProviderMutant.cpp'),
           '-L' + str(libraries), '-Wl,-rpath,' + str(libraries),
           '-lagiru_rt', '-lagiru_net', '-lagiru_al', '-lagiru_db', '-o', str(binary)]
subprocess.run(command, check=True)
result = subprocess.run([str(binary)], capture_output=True, text=True)
receipt = {'command': command, 'exit': result.returncode, 'stdout': result.stdout, 'stderr': result.stderr}
(artifacts / 'provider-control.json').write_text(json.dumps(receipt, indent=2) + '\n')
assert result.returncode == 1, receipt
assert 'missing live provider refuses' in result.stdout, receipt
print(json.dumps({'guard_no_op_mutant_exit': result.returncode, 'rejected': True}))
