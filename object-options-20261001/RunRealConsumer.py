from pathlib import Path
import hashlib
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
old = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
relative = Path('apps/base/system/environment/configuration/page/ReportSettings.cpp')
rows = []
for compiler, cxx in [('clang', 'clang++-19'), ('gcc', 'g++-14')]:
    for label, image in [('predecessor', old), ('current', source)]:
        command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                   '-fsyntax-only', '-I' + str(image / 'include')]
        command += ['-I' + str(image / 'apps' / app) for app in
                    ('base', 'system', 'foundation', 'shared', 'absent')]
        command.append(str(image / relative))
        result = subprocess.run(command, text=True, capture_output=True, timeout=90)
        valid = (result.returncode == 0 if label == 'current' else result.returncode != 0 and
                 'Refused' in result.stderr and 'Variable = T{};' in result.stderr)
        rows.append({'compiler': compiler, 'image': label, 'command': command,
                     'exit': result.returncode, 'diagnostics': result.stderr, 'valid': valid,
                     'consumer_sha256': hashlib.sha256((image / relative).read_bytes()).hexdigest()})
receipt = {'results': rows, 'success': all(row['valid'] for row in rows),
           'no_pch': True, 'hand_edited_apps': False, 'sql_or_full_saved_settings_workflow_proof': False,
           'private_helpers_not_called_by_fixture': True}
(root / 'artifacts/real-consumer.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'checks': len(rows), 'success': receipt['success'],
                  'exits': [row['exit'] for row in rows]}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
