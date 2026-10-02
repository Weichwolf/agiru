from pathlib import Path
import json

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
old_proof = Path('/home/cosmo/Git/agiru/build/native-catalogue-20261001/source/build/native-catalogue-proof')
script = (old_proof / 'LintComparison.py').read_text()
namespace = {'__file__': str(old_proof / 'LintComparison.py')}
prefix = script.split('\nreceipt = ', 1)[0]
exec(compile(prefix, str(old_proof / 'LintComparison.py'), 'exec'), namespace)
before_units = namespace['after_units']
after_units = dict(before_units)
for name in ('body-writer', 'text-gate'):
    after_units.update(namespace['units'](artifacts / f'{name}-targeted.log'))
before = set().union(*before_units.values())
after = set().union(*after_units.values())
receipt = {'normalization': 'path and diagnostic text; line numbers excluded',
           'baseline_units': len(before_units), 'carried_forward_units': len(after_units) - 2,
           'rechecked_units': ['src/gen/BodyWriter.cpp', 'test/gate/TextGate.cpp'],
           'before': len(before), 'after': len(after),
           'added': sorted(after - before), 'removed': sorted(before - after),
           'full_lint_green': False, 'baseline_or_suppression_increase': False}
(artifacts / 'lint-comparison.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt, indent=2))
