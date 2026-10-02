import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
main = root.parent.parent
libraries = main / 'build/llvm-20261001/source/build'
rows = []
for gate, checks in [('DataMeasure', 49), ('DataTable', 38)]:
    output = root / 'artifacts' / ('main-' + gate)
    command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '--rtlib=compiler-rt',
               '--unwindlib=libunwind', '-fuse-ld=lld-19', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-I' + str(main / 'include'), '-I' + str(main / 'test/gate'),
               str(main / 'test/gate' / (gate + 'Gate.cpp')), '-L' + str(libraries),
               '-Wl,-rpath,' + str(libraries), '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(output)]
    compiled = subprocess.run(command, capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    executed = subprocess.run([str(output)], capture_output=True, text=True)
    assert executed.returncode == 0 and f'{checks} check(s), 0 red' in executed.stdout
    rows.append({'gate': gate, 'command': command, 'compile_exit': compiled.returncode,
                 'exit': executed.returncode, 'stdout': executed.stdout})
(root / 'artifacts/main-gates.json').write_text(json.dumps(rows, indent=2) + '\n')
print(json.dumps([{'gate': row['gate'], 'stdout': row['stdout']} for row in rows]))
