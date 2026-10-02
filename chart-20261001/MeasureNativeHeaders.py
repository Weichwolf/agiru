from pathlib import Path
import difflib
import json
import re
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
rows = []
for path in sorted((source / 'apps').rglob('*.h')):
    relative = path.relative_to(source)
    previous = origin / relative
    if not previous.is_file() or path.read_bytes() == previous.read_bytes():
        continue
    before, after = previous.read_text(), path.read_text()
    old = set(re.findall(r'^#include "(platform/[^"\n]+\.h)"$', before, re.M))
    current = set(re.findall(r'^#include "(platform/[^"\n]+\.h)"$', after, re.M))
    body = re.sub(r'^#include[^\n]*\n', '', after, flags=re.M)
    inserted = [{'header': header, 'type_spelling_present': 'platform::' + Path(header).stem in body}
                for header in sorted(current - old)]
    if inserted:
        rows.append({'path': str(relative), 'added_native_includes': inserted})
receipt = {'headers': rows, 'changed_headers_with_native_additions': len(rows),
           'added_native_includes': sum(len(row['added_native_includes']) for row in rows),
           'added_includes_with_no_named_type': sum(not include['type_spelling_present']
               for row in rows for include in row['added_native_includes']),
           'spelling_audit_not_full_cxx_dependency_analysis': True}
label = sys.argv[1]
(root / 'artifacts' / (label + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key != 'headers'}), flush=True)
