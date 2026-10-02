from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
compiler = sys.argv[1]
if compiler not in ('clang', 'gcc'):
    raise SystemExit('expected clang|gcc')
build = source / ('build' if compiler == 'clang' else 'build/gcc')
cxx = 'clang++-19' if compiler == 'clang' else 'g++-14'
artifacts = root / 'artifacts'
executable = artifacts / f'emit-contracts-{compiler}'
emitter = Path('/home/cosmo/Git/agiru/build/native-contract-20261001/artifacts/EmitContracts.cpp')
flags = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror']
command = flags + ['-I' + str(source / 'src/al'), '-I' + str(source / 'src/gen'),
                   '-I' + str(source / 'include'), str(emitter), '-L' + str(build),
                   '-Wl,-rpath,' + str(build), '-lagiru_gen', '-lagiru_al', '-o', str(executable)]
compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
contracts = artifacts / f'contracts-{compiler}'
symbols = '/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH/src'
generated = (subprocess.run([str(executable), symbols, str(contracts)], text=True,
                            capture_output=True, timeout=60) if compiled.returncode == 0 else None)
rows = []
if generated is not None and generated.returncode == 0:
    for path in sorted(contracts.glob('*.cpp')):
        result = subprocess.run(flags + ['-fsyntax-only', '-I' + str(source / 'include'), str(path)],
                                text=True, capture_output=True, timeout=120)
        rows.append({'table': path.stem, 'exit': result.returncode,
                     'stdout': result.stdout, 'diagnostics': result.stderr})
receipt = {'compiler': compiler, 'emit_compile_command': command,
           'emit_compile_exit': compiled.returncode, 'emit_compile_stderr': compiled.stderr,
           'emit_execution_exit': generated.returncode if generated else None,
           'emit_stdout': generated.stdout if generated else None,
           'emit_stderr': generated.stderr if generated else None,
           'candidates': len(rows), 'matching': sum(row['exit'] == 0 for row in rows),
           'results': rows, 'complete_native_catalogue_proved': False}
(artifacts / f'contracts-{compiler}.json').write_text(json.dumps(receipt, indent=2) + '\n')
field = next((row for row in rows if row['table'] == 'Field'), None)
valid = len(rows) == 14 and field is not None and field['exit'] == 0
print(json.dumps({'compiler': compiler, 'candidates': len(rows), 'matching': receipt['matching'],
                  'Field': field['exit'] if field else None}), flush=True)
raise SystemExit(0 if valid else 1)
