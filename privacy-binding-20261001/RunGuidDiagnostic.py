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
artifacts = root / 'artifacts'
executable = artifacts / ('guid-filter-' + compiler)
command = ['clang++-19' if compiler == 'clang' else 'g++-14', '-std=c++23',
           '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I' + str(source / 'include'),
           '-I' + str(source / 'src/rt'), '-I' + str(source / 'test/gate'),
           str(root / 'GuidFilter.cpp'), '-L' + str(build), '-Wl,-rpath,' + str(build),
           '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(executable)]
compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
            if compiled.returncode == 0 else None)
receipt = {'compiler': compiler, 'command': command, 'compile_exit': compiled.returncode,
           'compile_stderr': compiled.stderr, 'execution_exit': executed.returncode if executed else None,
           'stdout': executed.stdout if executed else None, 'stderr': executed.stderr if executed else None,
           'defect_not_fixed': True, 'sql_parity_proved': False}
(artifacts / ('guid-filter-' + compiler + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
valid = (compiled.returncode == 0 and executed is not None and executed.returncode == 1
         and '2 check(s), 1 red' in executed.stdout)
print(json.dumps({'compiler': compiler, 'defect_reproduced': valid}), flush=True)
raise SystemExit(0 if valid else 1)
