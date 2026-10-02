import importlib.util
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(source)
apps = json.loads((source / 'apps.json').read_text())
includes = ['-I' + str(source / 'include'), '-I' + str(source / 'apps/absent'),
            '-I' + str(source / 'apps/shared')]
includes.extend('-I' + str(path) for path in sorted((source / 'apps').iterdir()) if path.is_dir())
flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
         '-Wno-infinite-recursion', '-fsyntax-only', *includes]
results = []
for name in ('PageFieldsSelectionList', 'PageFields', 'AddPageFields'):
    for suffix in ('.cpp', '.def.cpp'):
        path = source / 'apps/base/system/tooling/page' / (name + suffix)
        assert path.exists(), path
        result = subprocess.run(flags + [str(path)], capture_output=True, text=True)
        results.append({'source': str(path.relative_to(source)), 'exit': result.returncode,
                        'diagnostics': result.stderr})
after = verify.digest(source)
assert before == after
receipt = {'source_sha256': before, 'source_unchanged': True, 'no_PCH': True,
           'units': results, 'all_compile': all(row['exit'] == 0 for row in results),
           'workflow_or_live_provider_proved': False}
(artifacts / 'actual-pages.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'units': len(results), 'failed': sum(row['exit'] != 0 for row in results)}))
raise SystemExit(0 if receipt['all_compile'] else 1)
