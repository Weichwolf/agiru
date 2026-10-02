from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import importlib.util
import json
from pathlib import Path
import re
import subprocess

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identities = {str(tree): verify.digest(tree) for tree in (main, source)}
previous = json.loads((task.parent / 'native-record-context-integration-20261002/artifacts/consumers.json').read_text())
units = set(previous['units'])
call_sites = set()
for path in (source / 'apps').rglob('*.cpp'):
    assert path.read_bytes() == (main / path.relative_to(source)).read_bytes()
    if re.search(r'\b(?:FilterGroup|HasFilter)\s*\(\s*\)', path.read_text()):
        call_sites.add(str(path.relative_to(source)))
for unit in call_sites | {'apps/base/system/automation/page/WorkflowResponseFactBox.cpp'}:
    prefix = unit.removesuffix('.def.cpp').removesuffix('.cpp')
    for suffix in ('.cpp', '.def.cpp'):
        if (source / (prefix + suffix)).exists():
            units.add(prefix + suffix)
units = sorted(units)
commands = {}
for label, tree in [('before', main), ('after', source)]:
    flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic',
             '-Werror', '-ferror-limit=0', '-fsyntax-only', '-I' + str(tree / 'include'),
             '-I' + str(tree / 'apps/absent'), '-I' + str(tree / 'apps/shared')]
    flags.extend('-I' + str(path) for path in sorted((tree / 'apps').iterdir()) if path.is_dir())
    commands[label] = flags

def probe(pair):
    label, unit = pair
    tree = main if label == 'before' else source
    result = subprocess.run(commands[label] + [str(tree / unit)], capture_output=True, text=True)
    (artifacts / ('consumer-' + label + '-' + unit.replace('/', '-') + '.log')).write_text(result.stdout + result.stderr)
    row = dict(label=label, unit=unit, exit=result.returncode,
               errors=dict(Counter(re.findall(r': (?:fatal )?error: (.+)', result.stderr))))
    print(json.dumps(dict(label=label, unit=unit, exit=result.returncode)), flush=True)
    return row

with ThreadPoolExecutor(max_workers=2) as pool:
    rows = list(pool.map(probe, [(label, unit) for label in ('before', 'after') for unit in units]))
old = {row['unit']: row for row in rows if row['label'] == 'before'}
new = {row['unit']: row for row in rows if row['label'] == 'after'}
receipt = dict(source_hashes=identities,
    sources_unchanged=all(verify.digest(Path(tree)) == value for tree, value in identities.items()),
    units=units, emitted_parameterless_call_sites=sorted(call_sites), rows=rows, used_pch=False, jobs=2,
    before_green=sum(row['exit'] == 0 for row in old.values()),
    after_green=sum(row['exit'] == 0 for row in new.values()),
    lost=[unit for unit in units if old[unit]['exit'] == 0 and new[unit]['exit'] != 0],
    gained=[unit for unit in units if old[unit]['exit'] != 0 and new[unit]['exit'] == 0],
    new_diagnostics={unit: dict(Counter(new[unit]['errors']) - Counter(old[unit]['errors']))
                     for unit in units if Counter(new[unit]['errors']) - Counter(old[unit]['errors'])},
    runtime_SQL_or_full_tree_or_G1_proved=False)
(artifacts / 'consumers.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: receipt[key] for key in ('before_green', 'after_green', 'lost', 'gained', 'new_diagnostics')}))
assert receipt['sources_unchanged'] and not receipt['lost'] and not receipt['new_diagnostics']
