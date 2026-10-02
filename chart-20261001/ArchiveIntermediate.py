from pathlib import Path
import hashlib
import json
import shutil

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
archive = artifacts / 'before-native-header-fix'
archive.mkdir(exist_ok=False)
names = ('actual-chart.json', 'real-consumer.json', 'changes.json', 'prototype.patch',
         'lint-platformtables-comparison.json', 'lint-platformtables-targeted.log',
         'lint-codeunitwriter-comparison.json', 'header-cost.json',
         'controls-clang.json', 'controls-gcc.json', 'test.json', 'test.log',
         'test-gcc.json', 'test-gcc.log', 'gcc.json', 'transpile.json',
         'transpile-repeat.json', 'repeat-output.json', 'generated-clang.json',
         'generated-gcc.json', 'contracts-clang.json', 'contracts-gcc.json')
rows = []
for name in names:
    path = artifacts / name
    shutil.copy2(path, archive / name)
    rows.append({'path': name, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()})
(archive / 'manifest.json').write_text(json.dumps({
    'source_sha256': json.loads((archive / 'changes.json').read_text())['source_sha256'],
    'files': rows,
    'actual_chart_compile_failed': True,
    'actual_chart_execution_claims_in_original_receipt_are_invalid': True,
    'actual_chart_expected_checks_per_compiler': 12,
    'actual_chart_executed_checks_per_compiler': 0,
    'actual_chart_missing_checks_per_compiler': 12,
    'reason': 'Retain missing-header and independent DataMeasureType conversion diagnostics before rerunning.'
}, indent=2) + '\n')
print(json.dumps({'archived': len(rows), 'directory': str(archive)}), flush=True)
