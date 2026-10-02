import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
artifacts = task / 'artifacts'
prefix = sys.argv[1] + '-' if len(sys.argv) > 1 else ''
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identities = {str(tree): verify.digest(tree) for tree in (main, source)}
assert identities[str(main)] == json.loads((artifacts / 'origin.json').read_text())['origin_sha256']
rows = []
for label, tree in [('before', main), ('after', source)]:
    binary = artifacts / ('gate-control-' + label)
    command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '--rtlib=compiler-rt',
        '--unwindlib=libunwind', '-fuse-ld=lld-19', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
        '-I' + str(tree / 'include'), '-I' + str(tree / 'src/gen'), '-I' + str(tree / 'src/al'),
        '-I' + str(source / 'test/gate'), str(source / 'test/gate/GenSourceBindingGate.cpp'),
        '-L' + str(tree / 'build'), '-Wl,-rpath,' + str(tree / 'build'),
        '-lagiru_gen', '-lagiru_al', '-lagiru_rt', '-lagiru_net', '-lagiru_db',
        '-o', str(binary)]
    compiled = subprocess.run(command, capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    gate = subprocess.run([str(binary)], capture_output=True, text=True)
    (artifacts / (prefix + 'gate-control-' + label + '.log')).write_text(gate.stdout + gate.stderr)
    environment = dict(os.environ, B=str(tree / 'build'))
    test = subprocess.run(['python3', str(source / 'test/toolchain.py'), 'PageRecordBindingGate'],
        cwd=source, env=environment, capture_output=True, text=True)
    (artifacts / (prefix + 'python-control-' + label + '.log')).write_text(test.stdout + test.stderr)
    rows.append({'label': label, 'gate_exit': gate.returncode, 'python_exit': test.returncode})
receipt = {'source_hashes': identities, 'rows': rows,
    'sources_unchanged': all(verify.digest(Path(tree)) == value for tree, value in identities.items()),
    'old_runtime_and_public_headers_identical': all((main / relative).read_bytes() == (source / relative).read_bytes()
        for relative in verify.files(main) if relative.parts[0] == 'include' or relative.parts[:2] == ('src', 'rt'))}
(artifacts / (prefix + 'controls.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert receipt['sources_unchanged'] and receipt['old_runtime_and_public_headers_identical']
assert rows[0]['gate_exit'] != 0 and rows[0]['python_exit'] != 0
assert rows[1]['gate_exit'] == 0 and rows[1]['python_exit'] == 0
