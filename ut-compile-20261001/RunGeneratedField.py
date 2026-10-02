from pathlib import Path
import json
import os
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
compiler = sys.argv[1]
if compiler not in ('clang', 'gcc'):
    raise SystemExit('expected clang|gcc')
build = source / ('build' if compiler == 'clang' else 'build/gcc')
artifacts = root / 'artifacts'
fixture = artifacts / f'generated-field-{compiler}'
(fixture / 'source').mkdir(parents=True, exist_ok=True)
(fixture / 'apps.json').write_text(json.dumps({
    'apps': [{'name': 'fixture', 'source': 'source', 'depends': []}]}))
(fixture / 'scope.json').write_text(json.dumps({
    'include': ['Microsoft.Fixture'], 'exclude': []}))
(fixture / 'source/NativeField.Codeunit.al').write_bytes((root / 'NativeField.Codeunit.al').read_bytes())
symbols = '/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH'
transpile = [str(build / 'agirutc'), str(fixture), str(fixture / 'apps.json'),
             str(fixture / 'generated'), '--system-symbols', symbols]
generated = subprocess.run(transpile, text=True, capture_output=True, timeout=60)
units = list((fixture / 'generated').rglob('NativeField.cpp'))
executable = fixture / 'run'
command = ['clang++-19' if compiler == 'clang' else 'g++-14', '-std=c++23',
           '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I' + str(source / 'include'),
           '-I' + str(source / 'test/gate'), '-I' + str(fixture / 'generated/fixture'),
           '-I' + str(fixture / 'generated/shared'), str(root / 'GeneratedField.cpp')]
command += [str(path) for path in units]
command += ['-L' + str(build), '-Wl,-rpath,' + str(build),
            '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(executable)]
compiled = (subprocess.run(command, text=True, capture_output=True, timeout=120)
            if generated.returncode == 0 and len(units) == 1 else None)
executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
            if compiled is not None and compiled.returncode == 0 else None)
receipt = {'compiler': compiler, 'system_symbols': symbols, 'transpile_command': transpile,
           'transpile_exit': generated.returncode, 'transpile_stdout': generated.stdout,
           'transpile_stderr': generated.stderr, 'compile_command': command,
           'compile_exit': compiled.returncode if compiled else None,
           'compile_stdout': compiled.stdout if compiled else None,
           'compile_stderr': compiled.stderr if compiled else None,
           'execution_exit': executed.returncode if executed else None,
           'execution_stdout': executed.stdout if executed else None,
           'execution_stderr': executed.stderr if executed else None,
           'production_provider_proof': False, 'full_ut_proof': False}
(artifacts / f'generated-field-{compiler}.json').write_text(json.dumps(receipt, indent=2) + '\n')
valid = (generated.returncode == 0 and compiled is not None and compiled.returncode == 0
         and executed is not None and executed.returncode == 0)
print(json.dumps({'compiler': compiler, 'transpile_exit': generated.returncode,
                  'compile_exit': receipt['compile_exit'], 'execution_exit': receipt['execution_exit']}), flush=True)
raise SystemExit(0 if valid else 1)
