import importlib.util
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
parent = root.parent / 'page-table-field-20261001/source'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(source)
results = []
for label, image in (('previous', parent), ('current', source)):
    includes = ['-I' + str(image / 'include'), '-I' + str(image / 'apps/absent'),
                '-I' + str(image / 'apps/shared')]
    includes.extend('-I' + str(path) for path in sorted((image / 'apps').iterdir()) if path.is_dir())
    flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-Wno-infinite-recursion', '-fsyntax-only', *includes]
    for suffix in ('.cpp', '.def.cpp'):
        path = image / 'apps/base/system/security/user/page' / ('UserCard' + suffix)
        assert path.exists()
        result = subprocess.run(flags + [str(path)], capture_output=True, text=True)
        results.append({'implementation': label, 'source': str(path.relative_to(image)),
                        'exit': result.returncode, 'diagnostics': result.stderr})
after = verify.digest(source)
assert before == after
receipt = {'source_sha256': before, 'source_unchanged': True, 'no_PCH': True,
           'units': results, 'user_or_permission_workflow_proved': False}
(artifacts / 'actual-user.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'exits': [row['exit'] for row in results]}))
assert [row['exit'] for row in results] == [1, 0, 1, 0]
assert 'State_4' in results[0]['diagnostics']
assert results[0]['diagnostics'].count('error:') == 3
assert results[2]['diagnostics'].count('error:') == 1
assert 'State_4' not in results[2]['diagnostics']
assert 'IsWSKeyAllowed' in results[0]['diagnostics'] and 'IsWSKeyAllowed' in results[2]['diagnostics']
