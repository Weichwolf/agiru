from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
artifacts = root / 'artifacts'
compiler = sys.argv[1]
if compiler not in ('clang', 'gcc'):
    raise SystemExit('expected clang|gcc')
rows = []
for mode, image in (('old', origin), ('current', source)):
    build = image / ('build' if compiler == 'clang' else 'build/gcc')
    executable = artifacts / ('binding-' + compiler + '-' + mode)
    command = ['clang++-19' if compiler == 'clang' else 'g++-14', '-std=c++23',
               '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-I' + str(image / 'include'), '-I' + str(image / 'src/gen'),
               '-I' + str(image / 'src/al'), '-I' + str(source / 'test/gate'),
               str(source / 'test/gate/GenTableBindingGate.cpp'), '-L' + str(build),
               '-Wl,-rpath,' + str(build), '-lagiru_gen', '-lagiru_al', '-o', str(executable)]
    compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
    executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
                if compiled.returncode == 0 else None)
    rows.append({'image': mode, 'command': command, 'compile_exit': compiled.returncode,
                 'compile_stderr': compiled.stderr,
                 'execution_exit': executed.returncode if executed else None,
                 'stdout': executed.stdout if executed else None,
                 'stderr': executed.stderr if executed else None,
                 'whole_generator_not_partial_library_overlay': True})
(artifacts / ('binding-controls-' + compiler + '.json')).write_text(json.dumps(rows, indent=2) + '\n')
valid = all(row['compile_exit'] == 0 and row['execution_exit'] == (1 if row['image'] == 'old' else 0)
            for row in rows)
print(json.dumps({'compiler': compiler, 'valid': valid,
                  'outputs': [row['stdout'] for row in rows]}), flush=True)
raise SystemExit(0 if valid else 1)
