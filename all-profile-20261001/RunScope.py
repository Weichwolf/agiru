from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
overlay = artifacts / 'missing-option-header/platform'
overlay.mkdir(parents=True, exist_ok=True)
header = (source / 'include/platform/PersonalizationScope.h').read_text()
line = '#include "type/Option.h"\n'
if header.count(line) != 1:
    raise RuntimeError('negative control anchor differs')
(overlay / 'PersonalizationScope.h').write_text(header.replace(line, ''))
rows = []
for cxx in ('clang++-19', 'g++-14'):
    for mode in ('current', 'missing-option-declaration'):
        command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-fsyntax-only']
        if mode != 'current':
            command.append('-I' + str(overlay.parent))
        command += ['-I' + str(source / 'include'), str(root / 'ScopeOnly.cpp')]
        result = subprocess.run(command, text=True, capture_output=True, timeout=60)
        expected = 0 if mode == 'current' else 1
        valid = result.returncode == expected and (expected == 0 or 'OptionTraits' in result.stderr)
        rows.append({'compiler': cxx, 'mode': mode, 'command': command, 'exit': result.returncode,
                     'diagnostics': result.stderr, 'valid': valid})
(artifacts / 'scope-standalone.json').write_text(json.dumps(rows, indent=2) + '\n')
print(json.dumps({'checks': len(rows), 'failed': sum(not row['valid'] for row in rows)}), flush=True)
raise SystemExit(0 if all(row['valid'] for row in rows) else 1)
