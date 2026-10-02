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
executable = root / 'artifacts' / ('relation-probe-' + compiler)
command = ['clang++-19' if compiler == 'clang' else 'g++-14', '-std=c++23',
           '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I' + str(source / 'include'),
           '-I' + str(source / 'src/rt'), '-I' + str(source / 'test/gate'),
           str(root / 'RelationProbe.cpp'), '-L' + str(build), '-Wl,-rpath,' + str(build),
           '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(executable)]
compiled = subprocess.run(command, text=True, capture_output=True, timeout=90)
executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=20)
            if compiled.returncode == 0 else None)
receipt = {'compiler': compiler, 'compile_command': command,
           'compile_exit': compiled.returncode, 'compile_stderr': compiled.stderr,
           'execution_exit': executed.returncode if executed else None,
           'execution_stdout': executed.stdout if executed else None,
           'execution_stderr': executed.stderr if executed else None,
           'expected_checks': 11, 'expected_red': 4,
           'diagnostic_only': True, 'sql_or_navigation_proof': False,
           'source_relation_removed': False}
(root / 'artifacts' / ('relation-probe-' + compiler + '.json')).write_text(
    json.dumps(receipt, indent=2) + '\n')
valid = (compiled.returncode == 0 and executed is not None and executed.returncode == 1
         and 'QualifiedRecordLinkRelation: 11 check(s), 4 red' in executed.stdout)
print(json.dumps({key: receipt[key] for key in
                 ('compiler', 'compile_exit', 'execution_exit', 'expected_checks', 'expected_red')}),
      flush=True)
raise SystemExit(0 if valid else 1)
