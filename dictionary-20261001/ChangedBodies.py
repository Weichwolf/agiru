import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
trees = {'before': root.parent / 'page-source-binding-20261001/source', 'after': root / 'source'}
spec = importlib.util.spec_from_file_location('verify_snapshot', trees['after'] / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
source_before = verify.digest(trees['after'])
paths = [path.relative_to(trees['after']).as_posix()
         for path in (trees['after'] / 'apps').rglob('*.cpp')
         if path.read_bytes() != (trees['before'] / path.relative_to(trees['after'])).read_bytes()]
original = json.loads((artifacts / 'changed-bodies.json').read_text())
original_paths = {row['source'] for row in original['changed_bodies']}
paths = sorted(set(paths).union(original_paths))
rows = []
for path in sorted(paths):
    row = {'source': path, 'results': {}}
    for label, source in trees.items():
        includes = ['-I' + str(source / 'include'), '-I' + str(source / 'apps/absent'),
                    '-I' + str(source / 'apps/shared')]
        includes.extend('-I' + str(item) for item in sorted((source / 'apps').iterdir()) if item.is_dir())
        command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra',
                   '-Wpedantic', '-Werror', '-Wno-infinite-recursion', '-fsyntax-only',
                   *includes, str(source / path)]
        result = subprocess.run(command, capture_output=True, text=True)
        row['results'][label] = {'exit': result.returncode, 'diagnostics': result.stderr}
    rows.append(row)
    print(json.dumps({'source': path, 'exits': [row['results'][label]['exit'] for label in trees]}), flush=True)
receipt = {'changed_bodies': rows, 'original_population': len(paths), 'no_PCH': True,
           'source_sha256': source_before, 'source_unchanged': source_before == verify.digest(trees['after']),
           'retained_original_comparisons': sorted(original_paths),
           'new_compile_losses': [row['source'] for row in rows if row['results']['before']['exit'] == 0
                                  and row['results']['after']['exit'] != 0],
           'execution_SQL_or_G1_proved': False}
(artifacts / ('changed-bodies-' + sys.argv[1] + '.json' if len(sys.argv) > 1 else 'changed-bodies.json')).write_text(json.dumps(receipt, indent=2) + '\n')
assert receipt['source_unchanged'] and not receipt['new_compile_losses'], receipt['new_compile_losses']
