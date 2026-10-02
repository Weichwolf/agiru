from pathlib import Path
import hashlib
import json
import subprocess

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
artifacts.mkdir(exist_ok=True)
project = artifacts / 'al-oracle'
project.mkdir(exist_ok=True)
manifest = {'id': '7fc55ccf-552f-4a59-8c70-3f312503cbb2', 'name': 'Text Guid Oracle',
            'publisher': 'agiru', 'version': '1.0.0.0', 'runtime': '17.0',
            'platform': '28.0.0.0', 'idRanges': [{'from': 50199, 'to': 50199}]}
(project / 'app.json').write_text(json.dumps(manifest, indent=2) + '\n')
(project / 'Oracle.Codeunit.al').write_bytes((root / 'Oracle.Codeunit.al').read_bytes())
compiler = root / 'tools/al/alc'
symbols = Path('/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH')
rows = []
for name, options in [('positive', []), ('negative', ['/define:NEGATIVE']),
                      ('secret-variable-positive', ['/define:SECRET_ASSIGNMENT']),
                      ('secret-literal-negative', ['/define:SECRET_LITERAL']),
                      ('additional-types-negative', ['/define:ADDITIONAL_TYPES'])]:
    command = [str(compiler), '/project:' + str(project), '/packagecachepath:' + str(symbols),
               '/out:' + str(project / (name + '.app')), '/warnaserror+', '/parallel-', *options]
    result = subprocess.run(command, text=True, capture_output=True, timeout=60)
    diagnostics = result.stdout + result.stderr
    errors = [line for line in diagnostics.splitlines() if 'error AL' in line]
    comparison = name in ('negative', 'additional-types-negative')
    valid = (result.returncode == 0 if name in ('positive', 'secret-variable-positive') else
             result.returncode != 0 and len(errors) == (4 if comparison else 1) and
             all('Text' in line and ('Boolean' if comparison else 'SecretText') in line and
                 ('AL0175' if comparison else 'AL0122') in line for line in errors))
    rows.append({'name': name, 'command': command, 'exit': result.returncode,
                 'stdout': result.stdout, 'stderr': result.stderr, 'errors': errors, 'valid': valid})
join_project = artifacts / 'al-join-oracle'
join_project.mkdir(exist_ok=True)
(join_project / 'app.json').write_text(json.dumps(manifest, indent=2) + '\n')
(join_project / 'Join.Codeunit.al').write_bytes((root / 'Join.Codeunit.al').read_bytes())
command = [str(compiler), '/project:' + str(join_project), '/packagecachepath:' + str(symbols),
           '/out:' + str(join_project / 'join.app'), '/warnaserror+', '/parallel-']
result = subprocess.run(command, text=True, capture_output=True, timeout=60)
rows.append({'name': 'execution-fixture', 'command': command, 'exit': result.returncode,
             'stdout': result.stdout, 'stderr': result.stderr, 'valid': result.returncode == 0})
receipt = {'results': rows, 'success': all(row['valid'] for row in rows),
           'compiler_assembly_sha256': hashlib.sha256((root / 'tools/al/Microsoft.Dynamics.Nav.CodeAnalysis.dll').read_bytes()).hexdigest(),
           'compiler_version_output': subprocess.check_output([str(compiler), '/help'], text=True).splitlines()[0],
           'system_app_sha256': hashlib.sha256((symbols / 'System.app').read_bytes()).hexdigest(),
           'runtime_execution_proof': False}
(artifacts / 'al-oracle.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'compiler': receipt['compiler_version_output'], 'success': receipt['success'],
                  'results': [{'name': row['name'], 'exit': row['exit'], 'valid': row['valid']}
                              for row in rows]}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
