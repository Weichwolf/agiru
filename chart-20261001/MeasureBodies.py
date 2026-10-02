from pathlib import Path
import json
import re

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
scope = set(json.loads((root / 'artifacts/before-native-header-fix/changes.json').read_text())['generated_changed'])
rows = []
count = 0
for path in sorted((source / 'apps').rglob('*.cpp')):
    relative = str(path.relative_to(source))
    previous = origin / relative
    if not previous.is_file():
        raise SystemExit('generated body population changed: ' + relative)
    count += 1
    def tokens(owner):
        text = re.sub(r'^#include[^\n]*\n', '', owner.read_text(), flags=re.M)
        return re.findall(r'"(?:[^"\\]|\\.)*"|\w+|[^\s]', text)
    if tokens(previous) != tokens(path):
        rows.append(relative)
unexpected = sorted(set(rows) - scope)
receipt = {'cpp_files': count, 'changed_beyond_include_directives': rows,
           'allowed_original_chart_binding_paths': sorted(scope), 'unexpected': unexpected,
           'success': not unexpected,
           'scope_is_the_preserved_before_header_fix_chart_binding_receipt': True,
           'compilation_or_full_runtime_proof': False}
(root / 'artifacts/bodies.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'cpp_files': count, 'binding_body_changes': len(rows),
                  'unexpected': unexpected, 'success': not unexpected}), flush=True)
raise SystemExit(0 if not unexpected else 1)
