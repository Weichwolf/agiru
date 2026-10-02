from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
results = []
for tag, compiler in (('clang', 'clang++-19'), ('gcc', 'g++-14')):
    build = source / ('build' if tag == 'clang' else 'build/gcc')
    executable = artifacts / ('sort-' + tag)
    command = [compiler, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-I' + str(source / 'include'), '-I' + str(source / 'test/gate'),
               str(root / 'SortProbe.cpp'), '-L' + str(build), '-Wl,-rpath,' + str(build),
               '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(executable)]
    compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
    executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
                if compiled.returncode == 0 else None)
    results.append({'compiler': tag, 'compile_command': command, 'compile_exit': compiled.returncode,
                    'compile_stderr': compiled.stderr,
                    'execution_exit': executed.returncode if executed else None,
                    'execution_stdout': executed.stdout if executed else None,
                    'execution_stderr': executed.stderr if executed else None})
receipt = {'results': results, 'runtime_changed': False, 'sql_sort_proved': False,
           'success_claim': False, 'expected_reproduced_defect': all(
               row['compile_exit'] == 0 and row['execution_exit'] == 1 and
               'UnindexedProfileSort: 2 check(s), 1 red' in row['execution_stdout']
               for row in results)}
(artifacts / 'sort-result.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
raise SystemExit(0 if receipt['expected_reproduced_defect'] else 1)
