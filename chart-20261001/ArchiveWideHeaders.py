from pathlib import Path
import hashlib
import json
import shutil

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
archive = artifacts / 'before-native-local-guard'
archive.mkdir(exist_ok=False)
names = ('actual-chart.json', 'real-consumer.json', 'changes.json', 'prototype.patch',
         'lint-platformtables-comparison.json', 'lint-codeunitwriter-comparison.json',
         'lint-pagewriter-comparison.json', 'controls-clang.json', 'controls-gcc.json',
         'test.json', 'test.log', 'test-gcc.json', 'test-gcc.log', 'gcc.json',
         'transpile.json', 'transpile-repeat.json', 'repeat-output.json',
         'header-blast-before-local-guard.json', 'binder.json', 'binder.log')
rows = []
for name in names:
    path = artifacts / name
    shutil.copy2(path, archive / name)
    rows.append({'path': name, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()})
(archive / 'manifest.json').write_text(json.dumps({
    'source_sha256': json.loads((archive / 'changes.json').read_text())['source_sha256'],
    'files': rows,
    'header_blast_is_not_accepted_as_final_scope': True,
    'actual_chart_missing_checks_per_compiler': 12
}, indent=2) + '\n')
print(json.dumps({'archived': len(rows), 'directory': str(archive)}), flush=True)
