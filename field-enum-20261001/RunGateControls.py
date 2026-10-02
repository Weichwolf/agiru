from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
old = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
compiler = sys.argv[1]
if compiler not in ('clang', 'gcc'):
    raise SystemExit('expected clang|gcc')
cxx = 'clang++-19' if compiler == 'clang' else 'g++-14'
rows = []
for name, image in [('predecessor', old), ('current', source)]:
    build = image / ('build' if compiler == 'clang' else 'build/gcc')
    executable = root / 'artifacts' / ('binding-' + compiler + '-' + name)
    command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-I' + str(image / 'src/al'), '-I' + str(image / 'src/gen'),
               '-I' + str(image / 'include'), '-I' + str(source / 'test/gate'),
               str(source / 'test/gate/GenTableBindingGate.cpp'),
               '-L' + str(build), '-Wl,-rpath,' + str(build),
               '-lagiru_gen', '-lagiru_al', '-o', str(executable)]
    compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
    executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
                if compiled.returncode == 0 else None)
    valid = (compiled.returncode == 0 and executed is not None and
             executed.returncode == (1 if name == 'predecessor' else 0) and
             'GenTableBinding:' in executed.stdout and
             ('native field scopes do not invent an option-content hash' in executed.stdout
              if name == 'predecessor' else '0 red' in executed.stdout))
    rows.append({'image': name, 'command': command, 'compile_exit': compiled.returncode,
                 'compile_stderr': compiled.stderr,
                 'execution_exit': executed.returncode if executed else None,
                 'stdout': executed.stdout if executed else None,
                 'stderr': executed.stderr if executed else None, 'valid': valid,
                 'whole_generator_and_header_image': True})
(root / 'artifacts' / ('gate-controls-' + compiler + '.json')).write_text(json.dumps(rows, indent=2) + '\n')
print(json.dumps({'compiler': compiler, 'valid': all(row['valid'] for row in rows),
                  'outputs': [row['stdout'] for row in rows]}), flush=True)
raise SystemExit(0 if all(row['valid'] for row in rows) else 1)
