import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
trees = {'before': root.parent.parent, 'after': root / 'source'}
spec = importlib.util.spec_from_file_location('verify_snapshot', trees['after'] / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identities = {label: verify.digest(tree) for label, tree in trees.items()}
artifacts = root / 'artifacts'
program = trees['after'] / 'test/gate/ObjectCatalogueGate.cpp'
rows = []
for label, tree in trees.items():
    libraries = tree / 'build'
    binary = artifacts / ('object-catalogue-' + label)
    command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
               '-I' + str(tree / 'include'), '-I' + str(tree / 'test/gate'), str(program),
               '-L' + str(libraries), '-Wl,-rpath,' + str(libraries),
               '-lagiru_rt', '-lagiru_net', '-lagiru_al', '-lagiru_db', '-o', str(binary)]
    compiled = subprocess.run(command, capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    rows.append({'label': label, 'command': command, 'exit': result.returncode,
                 'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
                 'stdout': result.stdout, 'stderr': result.stderr})
    print(result.stdout + result.stderr, flush=True)
    assert result.returncode == (1 if label == 'before' else 0)
page = 'apps/system/system/reflection/page/Objects.cpp'
for label, tree in trees.items():
    includes = ['-I' + str(tree / 'include')]
    includes += ['-I' + str(app) for app in sorted((tree / 'apps').iterdir()) if app.is_dir()]
    result = subprocess.run(['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra',
                             '-Wpedantic', '-Werror', '-Wno-infinite-recursion', '-fsyntax-only',
                             *includes, str(tree / page)], capture_output=True, text=True)
    rows.append({'label': label, 'source': page, 'exit': result.returncode, 'diagnostics': result.stderr})
    print(json.dumps({'source': page, 'label': label, 'exit': result.returncode}), flush=True)
    assert result.returncode == (1 if label == 'before' else 0)
assert all(verify.digest(tree) == identities[label] for label, tree in trees.items())
(artifacts / 'controls.json').write_text(json.dumps({'source_hashes': identities, 'sources_unchanged': True,
                                                   'no_mixed_headers_libraries': True, 'rows': rows}, indent=2) + '\n')
