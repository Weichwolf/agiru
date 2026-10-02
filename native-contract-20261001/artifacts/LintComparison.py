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


before_units = units(old / 'build/lint/tidy.log')
for path in [old / 'build/system-table-proof/cli-main-targeted.log',
             old / 'build/system-table-proof/cli-binding-targeted.log']:
    before_units.update(units(path))
after_units = dict(before_units)
for stem in ['table-writer', 'codeunit-writer', 'page-writer', 'binder', 'native-object']:
    after_units.update(units(proof / (stem + '-targeted-final.log')))
before = set().union(*before_units.values())
after = set().union(*after_units.values())
receipt = {'normalization': 'path and diagnostic text; line numbers excluded',
           'before_units': len(before_units), 'after_units': len(after_units),
           'before': len(before), 'after': len(after),
           'added': sorted(after - before), 'removed': sorted(before - after),
           'scope': 'previous changed selection with final Main/binder checks, plus five new checks',
           'full_lint_green': False, 'baseline_or_suppression_increase': False}
(proof / 'lint-comparison.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt, indent=2))
