import hashlib
import json
from pathlib import Path
import subprocess
import sys

task = Path(__file__).resolve().parent
main = task.parent.parent
tree = main if sys.argv[1].startswith('before') else task / 'source'
label = sys.argv[1]
output = task / ('fixture-' + label)
artifacts = task / 'artifacts'
output.mkdir(exist_ok=True)
assert not list(output.iterdir()), 'refuse to mix fixture outputs'
commands = []

def run(stage, command):
    result = subprocess.run(command, cwd=tree, capture_output=True, text=True)
    (artifacts / f'fixture-{label}-{stage}.log').write_text(result.stdout + result.stderr)
    commands.append(dict(stage=stage, command=command, exit=result.returncode))
    return result

generated = run('generation', [str(tree / 'build/agirutc'), str(task / 'fixtures/al'),
    str(task / 'fixtures/apps.json'), str(output)])
if generated.returncode == 0:
    files = sorted(output.rglob('*.cpp'))
    compiled = run('compile', ['clang++-19', '-std=c++23', '-stdlib=libc++',
        '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
        '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I' + str(tree / 'include'),
        '-I' + str(output), '-I' + str(output / 'fixture'),
        '-I' + str(output / 'shared'), '-I' + str(output / 'absent'),
        *(str(path) for path in files), str(task / 'fixtures/Consumer.cpp'),
        '-L' + str(tree / 'build'), '-Wl,-rpath,' + str(tree / 'build'),
        '-lagiru_rt', '-lagiru_al', '-lagiru_net', '-lagiru_db',
        '-o', str(task / ('fixture-' + label + '-consumer'))])
    if compiled.returncode == 0:
        run('execution', [str(task / ('fixture-' + label + '-consumer'))])
receipt = dict(label=label, commands=commands,
    fixtures={str(path.relative_to(task / 'fixtures')): hashlib.sha256(path.read_bytes()).hexdigest()
        for path in (task / 'fixtures').rglob('*') if path.is_file()},
    generated={str(path.relative_to(output)): hashlib.sha256(path.read_bytes()).hexdigest()
        for path in output.rglob('*') if path.is_file()},
    used_pch=False, database_operations=False)
(artifacts / ('fixture-' + label + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'label': label, 'stages': [(row['stage'], row['exit']) for row in commands]}))
