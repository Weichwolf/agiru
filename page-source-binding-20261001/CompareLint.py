import json
from pathlib import Path
import re

root = Path(__file__).resolve().parent
parent = root.parent / 'page-table-field-20261001/source'
source = root / 'source'


def diagnostics(path):
    text = (path / 'build/lint/targeted.log').read_text()
    return {re.sub(r':\d+:\d+:', ':LOC:', line.replace(str(path), '<source>'))
            for line in text.splitlines() if ': error:' in line}


before, after = diagnostics(parent), diagnostics(source)
assert not after - before, after - before
assert not any("FieldsOfRecord' has cognitive complexity" in line for line in after)
assert not any("FieldsForTable' has cognitive complexity" in line for line in after)
receipt = {'target': 'src/gen/BodyWriter.cpp', 'baseline_findings': len(before),
           'current_findings': len(after), 'new_findings': sorted(after - before),
           'removed_findings': sorted(before - after), 'lint_green': not after,
           'initial_added_fields_complexity': 31,
           'refactored_fields_complexity_exceeds_25': False}
(root / 'artifacts/lint-comparison.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
