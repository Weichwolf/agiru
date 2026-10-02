from collections import Counter
import concurrent.futures
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
generation = json.loads((artifacts / 'generation.json').read_text())
units = ['apps/' + path for path in generation['changed'] if path.endswith('.cpp')]
units += ['apps/base/system/tooling/page/' + name + suffix
          for name in ('PageFieldsSelectionList', 'PageFields', 'AddPageFields')
          for suffix in ('.cpp', '.def.cpp')]
units = list(dict.fromkeys(units))
commands = {}
for label, tree in [('before', main), ('after', source)]:
    flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic',
             '-Werror', '-ferror-limit=0', '-fsyntax-only', '-I' + str(tree / 'include'),
             '-I' + str(tree / 'apps/absent'), '-I' + str(tree / 'apps/shared')]
    flags.extend('-I' + str(path) for path in sorted((tree / 'apps').iterdir()) if path.is_dir())
    commands[label] = flags

def compile_unit(item):
    label, unit = item
    tree = main if label == 'before' else source
    command = commands[label] + [str(tree / unit)]
    result = subprocess.run(command, capture_output=True, text=True)
    key = unit.replace('/', '-')
    (artifacts / ('consumer-' + label + '-' + key + '.log')).write_text(result.stdout + result.stderr)
    errors = Counter(re.findall(r': (?:fatal )?error: (.+)', result.stderr))
    return dict(label=label, unit=unit, exit=result.returncode, errors=dict(errors))

with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
    rows = list(pool.map(compile_unit, [(label, unit) for label in ('before', 'after') for unit in units]))
old = {row['unit']: row for row in rows if row['label'] == 'before'}
new = {row['unit']: row for row in rows if row['label'] == 'after'}
lost = [unit for unit in units if old[unit]['exit'] == 0 and new[unit]['exit'] != 0]
gained = [unit for unit in units if old[unit]['exit'] != 0 and new[unit]['exit'] == 0]
diagnostic_changes = []
for unit in units:
    added = Counter(new[unit]['errors']) - Counter(old[unit]['errors'])
    removed = Counter(old[unit]['errors']) - Counter(new[unit]['errors'])
    if added or removed:
        diagnostic_changes.append(dict(unit=unit, added=dict(added), removed=dict(removed)))
receipt = dict(source_hashes=identities,
    sources_unchanged=all(verify.digest(Path(tree)) == value for tree, value in identities.items()),
    used_pch=False, jobs=2, compiler_flags=commands, units=units, rows=rows,
    before_green=sum(row['exit'] == 0 for row in old.values()),
    after_green=sum(row['exit'] == 0 for row in new.values()),
    lost=lost, gained=gained, diagnostic_changes=diagnostic_changes,
    G1_proved=False, app_dependency_ownership_proved=False)
(artifacts / 'consumers.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: receipt[key] for key in ('before_green', 'after_green', 'lost', 'gained', 'diagnostic_changes')}))
assert receipt['sources_unchanged'] and not lost
