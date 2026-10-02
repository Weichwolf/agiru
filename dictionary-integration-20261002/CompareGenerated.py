import importlib.util
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
trees = {'before': root.parent.parent, 'after': root / 'source'}
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', trees['after'] / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identity = verify.digest(trees['after'])
generation = json.loads((artifacts / 'generation.json').read_text())


def body(path):
    return '\n'.join(line for line in path.read_text().splitlines()
                     if line.strip() and not line.lstrip().startswith('#include'))


paths = sorted(path for path in generation['changed'] if path.endswith('.cpp')
               and body(trees['before'] / 'apps' / path) != body(trees['after'] / 'apps' / path))
rows = []
for path in paths:
    row = {'source': path, 'results': {}}
    for label, source in trees.items():
        includes = ['-I' + str(source / 'include'), '-I' + str(source / 'apps/absent'),
                    '-I' + str(source / 'apps/shared')]
        includes.extend('-I' + str(item) for item in sorted((source / 'apps').iterdir()) if item.is_dir())
        command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic',
                   '-Werror', '-Wno-infinite-recursion', '-fsyntax-only', *includes,
                   str(source / 'apps' / path)]
        result = subprocess.run(command, capture_output=True, text=True)
        row['results'][label] = {'exit': result.returncode, 'diagnostics': result.stderr}
    rows.append(row)
    print(json.dumps({'source': path, 'exits': [row['results'][label]['exit'] for label in trees]}), flush=True)
losses = [row['source'] for row in rows if row['results']['before']['exit'] == 0
          and row['results']['after']['exit'] != 0]
receipt = {'source_sha256': identity, 'source_unchanged': identity == verify.digest(trees['after']),
           'cpp_files_with_body_changes': len(paths), 'rows': rows, 'compile_losses': losses,
           'include_only_changes_still_need_complete_tree_gate': True,
           'full_tree_or_G1_proved': False}
(artifacts / 'body-comparison.json').write_text(json.dumps(receipt, indent=2) + '\n')
assert receipt['source_unchanged'] and not losses, losses
