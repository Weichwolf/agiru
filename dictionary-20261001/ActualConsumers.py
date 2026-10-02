import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import time

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
label = sys.argv[1]
source = root / 'source' if label == 'new' else root.parent / 'page-source-binding-20261001/source'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(source)
includes = ['-I' + str(source / 'include'), '-I' + str(source / 'apps/absent'),
            '-I' + str(source / 'apps/shared')]
includes.extend('-I' + str(path) for path in sorted((source / 'apps').iterdir()) if path.is_dir())
flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
         '-Wno-infinite-recursion', '-fsyntax-only', *includes]
stems = ['apps/base/system/tooling/page/AddPageFields',
         'apps/base/system/automation/codeunit/RequestPageParametersHelper',
         'apps/base/system/utilities/codeunit/DictionaryWrapper']
results = []
for stem in stems:
    for suffix in (('.cpp', '.def.cpp') if '/page/' in stem else ('.cpp', '.h')):
        path = source / (stem + suffix)
        assert path.exists(), path
        started = time.monotonic()
        language = ['-x', 'c++-header'] if suffix == '.h' else []
        result = subprocess.run(flags + language + [str(path)], capture_output=True, text=True)
        results.append({'source': str(path.relative_to(source)), 'exit': result.returncode,
                        'elapsed_seconds': time.monotonic() - started, 'diagnostics': result.stderr})
        print(json.dumps({'source': str(path.relative_to(source)), 'exit': result.returncode,
                          'errors': result.stderr.count('error:')}), flush=True)
after = verify.digest(source)
assert before == after
receipt = {'source_sha256': before, 'source_unchanged': True, 'no_PCH': True,
           'units': results, 'all_six_units_retained': len(results) == 6,
           'execution_SQL_or_G1_proved': False}
(artifacts / ('actual-consumers-' + label + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
