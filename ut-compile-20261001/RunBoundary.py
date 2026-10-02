from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parent
current = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
mode, compiler = sys.argv[1:]
if mode not in ('current', 'old-image') or compiler not in ('clang', 'gcc'):
    raise SystemExit('expected current|old-image clang|gcc')
source = current if mode == 'current' else origin
build = source / ('build' if compiler == 'clang' else 'build/gcc')
artifacts = root / 'artifacts'
executable = artifacts / f'field-boundary-{mode}-{compiler}'
command = ['clang++-19' if compiler == 'clang' else 'g++-14', '-std=c++23',
           '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I' + str(source / 'include'),
           '-I' + str(source / 'test/gate'), str(root / 'FieldBoundary.cpp'),
           '-L' + str(build), '-Wl,-rpath,' + str(build),
           '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(executable)]
compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
            if compiled.returncode == 0 else None)
receipt = {'mode': mode, 'compiler': compiler, 'source': str(source), 'build': str(build),
           'command': command, 'compile_exit': compiled.returncode,
           'compile_stdout': compiled.stdout, 'compile_stderr': compiled.stderr,
           'execution_exit': executed.returncode if executed else None,
           'execution_stdout': executed.stdout if executed else None,
           'execution_stderr': executed.stderr if executed else None,
           'mixed_abi_images': False, 'database_provider_proof': False}
(artifacts / f'field-boundary-{mode}-{compiler}.json').write_text(
    json.dumps(receipt, indent=2) + '\n')
valid = compiled.returncode == 0 and executed is not None and (
    executed.returncode == (0 if mode == 'current' else 1))
print(json.dumps({'mode': mode, 'compiler': compiler, 'compile_exit': compiled.returncode,
                  'execution_exit': receipt['execution_exit'], 'expected_result': valid}), flush=True)
raise SystemExit(0 if valid else 1)
