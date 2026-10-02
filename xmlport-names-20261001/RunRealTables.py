from pathlib import Path
import json
import re
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
results = []
for compiler, build in (('clang', source / 'build'), ('gcc', source / 'build/gcc')):
    selected = re.search(r'^CMAKE_CXX_COMPILER:[^=]+=(.+)$',
                         (build / 'CMakeCache.txt').read_text(), re.M)
    if selected is None:
        raise RuntimeError('compiler provenance missing')
    sources = [str(source / 'apps/tests/core/table' / (stem + suffix))
               for stem in ('TestTableC_132512', 'TestTableC_139063')
               for suffix in ('.cpp', '.def.cpp')]
    executable = artifacts / f'real-tables-{compiler}'
    command = [selected[1], '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-I' + str(source / 'include'), '-I' + str(source / 'test/gate'),
               '-I' + str(source / 'apps/tests'), '-I' + str(source / 'apps/shared'),
               str(root / 'RealTables.cpp')] + sources + [
               '-L' + str(build), '-Wl,-rpath,' + str(build),
               '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(executable)]
    compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
    executed = subprocess.run([str(executable)], text=True, capture_output=True, timeout=30) if compiled.returncode == 0 else None
    receipt = {'compiler': compiler, 'command': command, 'compile_exit': compiled.returncode,
               'compile_stdout': compiled.stdout, 'compile_stderr': compiled.stderr,
               'execution_exit': executed.returncode if executed else None,
               'execution_stdout': executed.stdout if executed else None,
               'execution_stderr': executed.stderr if executed else None,
               'database_execution_proof': False, 'flowfield_execution_proof': False}
    (artifacts / f'real-tables-{compiler}.json').write_text(json.dumps(receipt, indent=2) + '\n')
    success = (compiled.returncode == 0 and executed.returncode == 0 and
               'RealTableIdentity: 20 check(s), 0 red' in executed.stdout)
    results.append(success)
    print(json.dumps({'compiler': compiler, 'compile_exit': compiled.returncode,
                      'execution_exit': receipt['execution_exit'], 'success': success}), flush=True)
raise SystemExit(0 if all(results) else 1)
