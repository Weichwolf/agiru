from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
old = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
fixture = root / 'artifacts/generated-predecessor'
(fixture / 'source').mkdir(parents=True, exist_ok=True)
(fixture / 'apps.json').write_text(json.dumps({'apps': [{'name': 'fixture', 'source': 'source', 'depends': []}]}))
(fixture / 'scope.json').write_text(json.dumps({'include': ['Microsoft.Fixture'], 'exclude': []}))
for name in ('StoredFlag.Table.al', 'SavedOptions.Codeunit.al'):
    (fixture / 'source' / name).write_bytes((root / name).read_bytes())
symbols = '/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH'
generated = subprocess.run([str(old / 'build/agirutc'), str(fixture), str(fixture / 'apps.json'),
                            str(fixture / 'generated'), '--system-symbols', symbols],
                           text=True, capture_output=True, timeout=60)
units = sorted((fixture / 'generated').rglob('*.cpp'))
rows = []
for cxx in ('clang++-19', 'g++-14'):
    command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-fsyntax-only', '-I' + str(old / 'include'),
               '-I' + str(fixture / 'generated/fixture'), '-I' + str(fixture / 'generated/shared'),
               '-I' + str(fixture / 'generated/absent')]
    command += [str(path) for path in units]
    result = subprocess.run(command, text=True, capture_output=True, timeout=120)
    rows.append({'compiler': cxx, 'command': command, 'exit': result.returncode,
                 'diagnostics': result.stderr, 'valid': result.returncode != 0 and
                 'Refused' in result.stderr and 'Temporary' in result.stderr})
receipt = {'transpile_exit': generated.returncode, 'stdout': generated.stdout,
           'stderr': generated.stderr, 'units': [str(path) for path in units], 'results': rows,
           'whole_predecessor_generator_and_headers': True,
           'success': generated.returncode == 0 and len(units) == 3 and all(row['valid'] for row in rows)}
(root / 'artifacts/generated-controls.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'success': receipt['success'], 'exits': [row['exit'] for row in rows]}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
