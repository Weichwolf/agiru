import json
from pathlib import Path
import re

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
rows = json.loads((artifacts / 'lint-immutable-binder-comparison.json').read_text())
assert len(rows) == 6
inherited = {line for row in rows for line in row['results'].get('before', {}).get('findings', [])
             if line.startswith('<source>/include/')}
text = (source / 'build/lint/targeted.log').read_text()
assert 'DictionaryGate.cpp' in text
(artifacts / 'lint-final-dictionary.log').write_text(text)
current = {re.sub(r':\d+:\d+:', ':LOC:', line.replace(str(source), '<source>'))
           for line in text.splitlines() if ': error:' in line}
assert len(current) == 20 and current.issubset(inherited), current - inherited
for row in rows:
    if row['unit'] == 'test/gate/DictionaryGate.cpp':
        row['results']['after'] = {'exit': 2, 'findings': sorted(current)}
        row['inherited_header_findings'] = sorted(current)
        row['new_findings'] = []
        row['refresh'] = 'lint-final-dictionary.log; only this gate changed after the six-unit run'
assert all(not row['new_findings'] for row in rows)
(artifacts / 'lint-final-comparison.json').write_text(json.dumps(rows, indent=2) + '\n')
print(json.dumps({'units': len(rows), 'new_findings': 0, 'dictionary_inherited_header_findings': 20}))
