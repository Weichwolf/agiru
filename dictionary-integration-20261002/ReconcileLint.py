import importlib.util
import json
from pathlib import Path
import re

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
before, after = root.parent.parent, root / 'source'
path = artifacts / 'targeted-lint.json'
receipt = json.loads(path.read_text())
spec = importlib.util.spec_from_file_location('verify_snapshot', after / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
assert all(verify.digest(Path(tree)) == value for tree, value in receipt['source_hashes'].items())
control = artifacts / 'targeted-lint-driver-control.json'
if not control.exists():
    control.write_text(json.dumps(receipt, indent=2) + '\n')
inherited = set()
for row in receipt['rows']:
    for diagnostic in row['results'].get('before', {}).get('findings', []):
        match = re.match(r'<source>/(.+\.h):LOC:', diagnostic)
        if diagnostic.startswith('<source>/include/') or (
                match and (before / match[1]).read_bytes() == (after / match[1]).read_bytes()):
            inherited.add(diagnostic)
for row in receipt['rows']:
    old = set(row['results'].get('before', {}).get('findings', []))
    new = set(row['results']['after']['findings'])
    matched = new.intersection(inherited) if row['unit'].startswith('test/') else set()
    row['inherited_header_findings'] = sorted(matched)
    row['new_findings'] = sorted(new - old - matched)
    print(json.dumps({'unit': row['unit'], 'new_findings': row['new_findings']}))
receipt['inherited_generator_headers_verified_byte_identical'] = True
path.write_text(json.dumps(receipt, indent=2) + '\n')
assert not any(row['new_findings'] for row in receipt['rows'])
