from pathlib import Path
import json
import re

old = Path('/home/cosmo/Git/agiru-worktrees/goal-20260928')
proof = Path(__file__).resolve().parent
section = re.compile(r'^== (.+?) \(exit \d+\) ==$')
diagnostic = re.compile(r'^.+?/(include|src|test)/(.+?):\d+:\d+: (?:error|warning): (.+)$')


def units(path):
    result = {}
    current = None
    for line in path.read_text().splitlines():
        match = section.match(line)
        if match:
            current = match[1]
            result[current] = set()
        match = diagnostic.match(line)
        if match and current is not None:
            result[current].add((match[1] + '/' + match[2], match[3]))
    if not result:
        raise RuntimeError('missing analyzed units: ' + str(path))
    return result


native = proof.parent / 'native-contract-proof'
before_units = units(old / 'build/lint/tidy.log')
for path in [old / 'build/system-table-proof/cli-main-targeted.log',
             old / 'build/system-table-proof/cli-binding-targeted.log']:
    before_units.update(units(path))
for stem in ['table-writer', 'codeunit-writer', 'page-writer', 'binder', 'native-object']:
    before_units.update(units(native / (stem + '-targeted-final.log')))
after_units = dict(before_units)
for stem in ['table-writer', 'key-gate']:
    after_units.update(units(proof / (stem + '-targeted.log')))
before = set().union(*before_units.values())
after = set().union(*after_units.values())
improved = []
added = after - before
removed = before - after
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
           'before_units': len(before_units), 'after_units': len(after_units),
           'before': len(before), 'after': len(after),
           'added': sorted(added), 'removed': sorted(removed),
           'improved_complexities': improved, 'new_failure_modes': sorted(new_failures),
           'scope': 'previous 180-unit selection plus final key writer/gate checks',
           'full_lint_green': False, 'baseline_or_suppression_increase': False}
(proof / 'lint-comparison.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt, indent=2))
