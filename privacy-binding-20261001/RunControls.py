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
flags = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror']
artifacts = root / 'artifacts'
rows = []
for label, image in [('old', old), ('current', source)]:
    build = image / ('build' if compiler == 'clang' else 'build/gcc')
    executable = artifacts / ('native-' + compiler + '-' + label)
    command = flags + ['-I' + str(image / 'include'), '-I' + str(source / 'test/gate'),
                       str(source / 'test/gate/NativeObjectGate.cpp'),
                       '-L' + str(build), '-Wl,-rpath,' + str(build),
                       '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(executable)]
    compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
    executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
                if compiled.returncode == 0 else None)
    rows.append({'image': label, 'command': command, 'compile_exit': compiled.returncode,
                 'compile_stderr': compiled.stderr,
                 'execution_exit': executed.returncode if executed else None,
                 'stdout': executed.stdout if executed else None,
                 'stderr': executed.stderr if executed else None,
                 'whole_image_not_mixed_abi': True})
(artifacts / ('controls-' + compiler + '.json')).write_text(json.dumps(rows, indent=2) + '\n')
valid = all(row['compile_exit'] == 0 and row['execution_exit'] == (1 if row['image'] == 'old' else 0)
            for row in rows)
print(json.dumps({'compiler': compiler, 'valid': valid,
                  'outputs': [row['stdout'] for row in rows]}), flush=True)
raise SystemExit(0 if valid else 1)
