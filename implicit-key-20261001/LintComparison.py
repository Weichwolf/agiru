from pathlib import Path
import json
import re

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
previous = Path('/home/cosmo/Git/agiru/build/text-result-20261001/LintComparison.py')
script = previous.read_text()
namespace = {'__file__': str(previous)}
prefix = script.split('\nreceipt = ', 1)[0]
exec(compile(prefix, str(previous), 'exec'), namespace)
before_units = dict(namespace['after_units'])
before_units.update(namespace['namespace']['units'](artifacts / 'parser-before-targeted.log'))
after_units = dict(before_units)
for name in ('parser', 'table-writer', 'tc-main', 'key-gate'):
    after_units.update(namespace['namespace']['units'](artifacts / f'{name}-targeted.log'))
before = set().union(*before_units.values())
after = set().union(*after_units.values())
added, removed = after - before, before - after
improved = []
complexity = re.compile(r"^(function '.+' has cognitive complexity of )([0-9]+)( .+)$")
for path, message in added:
    current = complexity.fullmatch(message)
    if current is None:
        continue
    for old_path, old_message in removed:
        previous = complexity.fullmatch(old_message)
        if old_path == path and previous is not None and current[1] == previous[1] and current[3] == previous[3] and int(current[2]) < int(previous[2]):
            improved.append({'path': path, 'before': int(previous[2]), 'after': int(current[2]),
                             'old_message': old_message, 'new_message': message})
new_failures = added - {(entry['path'], entry['new_message']) for entry in improved}
receipt = {'normalization': 'path and diagnostic text; line numbers excluded',
           'previous_carried_selection_units': len(namespace['after_units']),
           'selection_extended_units': ['src/al/Parser.cpp'],
           'baseline_units': len(before_units), 'carried_forward_units': len(after_units) - 4,
           'rechecked_units': ['src/al/Parser.cpp', 'src/gen/TableWriter.cpp',
                              'src/tc/Main.cpp', 'test/gate/GenKeyGate.cpp'],
           'before': len(before), 'after': len(after),
           'added': sorted(added), 'removed': sorted(removed),
           'improved_complexities': improved, 'new_failure_modes': sorted(new_failures),
           'full_lint_green': False, 'baseline_or_suppression_increase': False}
(artifacts / 'lint-comparison.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt, indent=2))
