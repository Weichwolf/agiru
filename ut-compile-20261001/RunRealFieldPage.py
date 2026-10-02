from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
artifacts = root / 'artifacts'
rows = []
for tag, cxx in (('clang', 'clang++-19'), ('gcc', 'g++-14')):
    for mode, headers in (('current', source), ('old-headers', origin)):
        for suffix in ('.cpp', '.def.cpp'):
            path = source / ('apps/system/system/privacy/page/FieldDataClassification' + suffix)
            command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                       '-fsyntax-only', '-I' + str(headers / 'include'),
                       '-I' + str(source / 'apps/system'), '-I' + str(source / 'apps/shared'), str(path)]
            result = subprocess.run(command, text=True, capture_output=True, timeout=120)
            expected = 1 if mode == 'old-headers' and suffix == '.def.cpp' else 0
            valid = result.returncode == expected
            if expected != 0:
                valid = valid and 'native field' in result.stderr
            rows.append({'compiler': tag, 'mode': mode, 'unit': str(path), 'command': command,
                         'expected_exit': expected, 'exit': result.returncode,
                         'diagnostics': result.stderr, 'valid': valid})
receipt = {'results': rows, 'success': all(row['valid'] for row in rows),
           'execution_proof': False, 'browser_proof': False, 'mixed_abi_execution': False}
(artifacts / 'real-field-page.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'checks': len(rows), 'failed_checks': sum(not row['valid'] for row in rows)}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
