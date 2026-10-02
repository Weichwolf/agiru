from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import time

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
trees = {'before': root.parent.parent, 'after': root / 'source'}
spec = importlib.util.spec_from_file_location('verify_snapshot', trees['after'] / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identities = {label: verify.digest(tree) for label, tree in trees.items()}
origin = json.loads((artifacts / 'source-identity.json').read_text())
assert identities['before'] == origin['origin_sha256'], origin
population = json.loads((artifacts / 'affected-sources.json').read_text())
units = population['quoted_include_transitive_cpp']
assert len(units) == 965 and len(set(units)) == len(units)
prefixes = {}
for label, tree in trees.items():
    prefixes[label] = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra',
                       '-Wpedantic', '-Werror', '-Wno-infinite-recursion', '-fsyntax-only',
                       '-I' + str(tree / 'include')]
    prefixes[label] += ['-I' + str(path) for path in sorted((tree / 'apps').iterdir()) if path.is_dir()]
logs = artifacts / 'consumers'
logs.mkdir(exist_ok=True)
started = time.monotonic()

def measure(unit):
    assert (trees['before'] / unit).read_bytes() == (trees['after'] / unit).read_bytes(), unit
    row = {'unit': unit, 'source_sha256': hashlib.sha256((trees['after'] / unit).read_bytes()).hexdigest()}
    for label, tree in trees.items():
        begin = time.monotonic()
        result = subprocess.run([*prefixes[label], str(tree / unit)], cwd=tree, capture_output=True, text=True)
        log = logs / (unit.replace('/', '__') + '.' + label + '.log')
        log.write_text(result.stdout + result.stderr)
        row[label] = {'exit': result.returncode, 'elapsed_seconds': time.monotonic() - begin,
                      'log': str(log.relative_to(root))}
    return row

rows = []
with ThreadPoolExecutor(max_workers=4) as pool:
    work = [pool.submit(measure, unit) for unit in units]
    for future in as_completed(work):
        rows.append(future.result())
        if len(rows) % 25 == 0:
            print(json.dumps({'measured': len(rows), 'total': len(units),
                              'losses': sum(row['before']['exit'] == 0 and row['after']['exit'] != 0 for row in rows)}), flush=True)
receipt = {'source_hashes': identities, 'sources_unchanged': all(verify.digest(tree) == identities[label] for label, tree in trees.items()),
           'scope': 'all 965 generated C++ consumers in the quoted-include closure; no PCH, unchanged source bodies',
           'full_generated_tree_passed': False, 'elapsed_seconds': time.monotonic() - started,
           'prefixes': prefixes, 'rows': sorted(rows, key=lambda row: row['unit']),
           'gains': [row['unit'] for row in rows if row['before']['exit'] != 0 and row['after']['exit'] == 0],
           'losses': [row['unit'] for row in rows if row['before']['exit'] == 0 and row['after']['exit'] != 0],
           'before_green': sum(row['before']['exit'] == 0 for row in rows),
           'after_green': sum(row['after']['exit'] == 0 for row in rows)}
(artifacts / 'consumer-comparison.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key not in ('rows', 'prefixes')}), flush=True)
assert receipt['sources_unchanged'] and not receipt['losses']
