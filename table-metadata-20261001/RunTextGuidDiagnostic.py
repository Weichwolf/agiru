from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
rows = []
for compiler, cxx in [('clang', 'clang++-19'), ('gcc', 'g++-14')]:
    for label, image, unit in [('predecessor-reduced', origin, root / 'TextGuidProbe.cpp'),
                               ('current-reduced', source, root / 'TextGuidProbe.cpp'),
                               ('actual-consumer', source, source / 'apps/base/finance/consolidation/codeunit/ImportConsolidationFromAPI.cpp')]:
        command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-fsyntax-only',
                   '-I' + str(image / 'include')]
        command += ['-I' + str(source / 'apps' / app) for app in
                    ('base', 'system', 'foundation', 'shared', 'absent')]
        command.append(str(unit))
        result = subprocess.run(command, text=True, capture_output=True, timeout=90)
        valid = (result.returncode != 0 and 'Guid.h:' in result.stderr and
                 '226' in result.stderr and 'return left + right.ToText();' in result.stderr)
        rows.append({'compiler': compiler, 'image': label, 'command': command,
                     'exit': result.returncode, 'diagnostics': result.stderr, 'valid': valid})
receipt = {'results': rows, 'success': all(row['valid'] for row in rows),
           'diagnostic_only': True, 'no_pch': True, 'execution_proof': False,
           'business_source_changed': False}
(root / 'artifacts/text-guid-diagnostic.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'checks': len(rows), 'valid': receipt['success']}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
