import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
rows = []
for name, includes in [('old', root.parent / 'chart-20261001/source/include'),
                       ('current', source / 'include'),
                       ('main', root.parent.parent / 'include')]:
    executable = artifacts / ('numeric-' + name)
    command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '--rtlib=compiler-rt',
               '--unwindlib=libunwind', '-fuse-ld=lld-19', '-Wall', '-Wextra',
               '-Wpedantic', '-Werror', '-I' + str(includes),
               '-I' + str(source / 'test/gate'), str(root / 'NumericControl.cpp'),
               '-o', str(executable)]
    compiled = subprocess.run(command, text=True, capture_output=True)
    assert compiled.returncode == 0, compiled.stderr
    executed = subprocess.run([str(executable)], text=True, capture_output=True)
    rows.append({'name': name, 'command': command, 'compile_exit': compiled.returncode,
                 'exit': executed.returncode, 'stdout': executed.stdout, 'stderr': executed.stderr})
assert rows[0]['exit'] != 0 and '7 check(s), 5 red' in rows[0]['stdout']
assert rows[1]['exit'] == 0 and '7 check(s), 0 red' in rows[1]['stdout']
assert rows[2]['exit'] == 0 and '7 check(s), 0 red' in rows[2]['stdout']
(artifacts / 'numeric-controls.json').write_text(json.dumps(rows, indent=2) + '\n')
print(json.dumps([{'name': row['name'], 'exit': row['exit'], 'stdout': row['stdout']} for row in rows]))
