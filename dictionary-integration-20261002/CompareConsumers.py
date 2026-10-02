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
prototype = json.loads((root.parent / 'dictionary-20261001/artifacts/actual-consumers-new.json').read_text())
paths = [row['source'] for row in prototype['units']]
paths += ['apps/system/system/tooling/codeunit/ProfilingDataProcessor.cpp',
          'apps/tests/core/table/AsmAvailabilityTestBuffer.cpp']
rows = []
for path in paths:
    row = {'source': path, 'results': {}}
    for label, tree in trees.items():
        includes = ['-I' + str(tree / 'include')]
        includes.extend('-I' + str(app) for app in sorted((tree / 'apps').iterdir()) if app.is_dir())
        command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic',
                   '-Werror', '-Wno-infinite-recursion', '-fsyntax-only', '-x',
                   'c++-header' if path.endswith('.h') else 'c++',
                   *includes, str(tree / path)]
        result = subprocess.run(command, capture_output=True, text=True)
        row['results'][label] = {'exit': result.returncode, 'diagnostics': result.stderr}
    rows.append(row)
    print(json.dumps({'source': path, 'exits': [row['results'][label]['exit'] for label in trees]}), flush=True)
losses = [row['source'] for row in rows if row['results']['before']['exit'] == 0
          and row['results']['after']['exit'] != 0]
receipt = {'source_hashes': identities, 'rows': rows, 'compile_losses': losses,
           'sources_unchanged': all(verify.digest(tree) == identities[label] for label, tree in trees.items()),
           'no_PCH': True, 'execution_SQL_or_G1_proved': False}
(root / 'artifacts/consumer-comparison.json').write_text(json.dumps(receipt, indent=2) + '\n')
assert receipt['sources_unchanged'] and not losses
