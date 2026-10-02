from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
rows = []
for compiler in ('clang++-19', 'g++-14'):
    command = [compiler, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-fsyntax-only', '-I' + str(root / 'source/include'),
               str(root / 'SecretAssignmentProbe.cpp')]
    result = subprocess.run(command, text=True, capture_output=True, timeout=60)
    rows.append({'command': command, 'exit': result.returncode, 'diagnostics': result.stderr})
oracle = json.loads((root / 'artifacts/al-oracle.json').read_text())
negative = next(row for row in oracle['results'] if row['name'] == 'secret-literal-negative')
positive = next(row for row in oracle['results'] if row['name'] == 'secret-variable-positive')
receipt = {'cpp_results': rows, 'al_result': negative,
           'al_variable_result': positive,
           'success': negative['valid'] and positive['valid'] and all(row['exit'] == 0 for row in rows),
           'diagnostic_only': True, 'fixed': False,
           'plain_text_variable_is_supported': True, 'literal_assignment_requires_distinct_validation': True}
(root / 'artifacts/secret-assignment-diagnostic.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'success': receipt['success'], 'fixed': False}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
