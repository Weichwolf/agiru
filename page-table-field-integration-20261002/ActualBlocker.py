import importlib.util
import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
main = task.parent.parent
before = task.parent / 'object-catalogue-integration-20261002/source'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identities = {str(tree): verify.digest(tree) for tree in (before, main)}
rows = []
for label, source in [('before', before), ('after', main)]:
    includes = ['-I' + str(source / 'include'), '-I' + str(source / 'apps/absent'),
                '-I' + str(source / 'apps/shared')]
    includes.extend('-I' + str(path) for path in sorted((source / 'apps').iterdir()) if path.is_dir())
    flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-Wno-infinite-recursion', '-fsyntax-only', *includes]
    for name in ('PageFieldsSelectionList', 'PageFields', 'AddPageFields'):
        for suffix in ('.cpp', '.def.cpp'):
            relative = 'apps/base/system/tooling/page/' + name + suffix
            assert (source / relative).read_bytes() == (main / relative).read_bytes()
            result = subprocess.run(flags + [str(source / relative)], capture_output=True, text=True)
            rows.append(dict(label=label, unit=relative, exit=result.returncode, diagnostics=result.stderr))
for old, new in zip(rows[:6], rows[6:]):
    assert old['unit'] == new['unit'] and old['exit'] == new['exit']
    assert old['diagnostics'].replace(str(before), '<source>') == new['diagnostics'].replace(str(main), '<source>')
assert rows[6]['exit'] != 0 and 'Format(Caption)' in rows[6]['diagnostics']
receipt = dict(source_hashes=identities, no_PCH=True, rows=rows,
               source_unchanged=all(verify.digest(Path(tree)) == value for tree, value in identities.items()),
               compiler_blocker_fixed=False, generated_tree_or_G1_proved=False)
(task / 'artifacts/actual-blocker.json').write_text(json.dumps(receipt, indent=2) + '\n')
assert receipt['source_unchanged']
print(json.dumps({'units_compared': 6, 'status_or_diagnostic_changes': 0, 'blocker_fixed': False}))
