from pathlib import Path
import hashlib
import json
import subprocess

root = Path(__file__).resolve().parent
project = root / 'artifacts/al-oracle'
project.mkdir(parents=True, exist_ok=True)
(project / 'app.json').write_text(json.dumps({
    'id': '47ef7bd1-5a07-41c8-a667-4049a957557a', 'name': 'Field Option Oracle',
    'publisher': 'agiru', 'version': '1.0.0.0', 'runtime': '17.0',
    'platform': '28.0.0.0', 'idRanges': [{'from': 50221, 'to': 50222}]}))
for name in ('Option.Table.al', 'Option.Page.al'):
    (project / name).write_bytes((root / name).read_bytes())
compiler = root.parent / 'text-guid-20261001/tools/al/alc'
symbols = Path('/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH')
command = [str(compiler), '/project:' + str(project), '/packagecachepath:' + str(symbols),
           '/out:' + str(project / 'options.app'), '/warnaserror+', '/parallel-']
result = subprocess.run(command, text=True, capture_output=True, timeout=60)
negative = root / 'artifacts/al-option-method-negative'
negative.mkdir(parents=True, exist_ok=True)
(negative / 'app.json').write_text(json.dumps({
    'id': '64680b83-6316-4461-a993-baad6fa38283', 'name': 'Option Method Refusal',
    'publisher': 'agiru', 'version': '1.0.0.0', 'runtime': '17.0',
    'platform': '28.0.0.0', 'idRanges': [{'from': 50223, 'to': 50223}]}))
(negative / 'Invalid.Codeunit.al').write_text('''namespace Microsoft.Fixture;
using System.Reflection;
codeunit 50223 Invalid {
  procedure Read(var Row: Record AllObjWithCaption): Integer
  begin exit(Row."Object Type".AsInteger()); end;
}
''')
negative_command = [str(compiler), '/project:' + str(negative),
                    '/packagecachepath:' + str(symbols), '/out:' + str(negative / 'invalid.app'),
                    '/warnaserror+', '/parallel-']
refused = subprocess.run(negative_command, text=True, capture_output=True, timeout=60)
errors = [line for line in (refused.stdout + refused.stderr).splitlines() if 'error AL' in line]
valid = (result.returncode == 0 and refused.returncode != 0 and len(errors) == 1 and
         'AL0132' in errors[0] and "'Option'" in errors[0] and 'AsInteger' in errors[0])
receipt = {'command': command, 'exit': result.returncode,
           'stdout': result.stdout, 'stderr': result.stderr,
           'negative': {'command': negative_command, 'exit': refused.returncode,
                        'stdout': refused.stdout, 'stderr': refused.stderr, 'errors': errors},
           'success': valid,
           'compiler_assembly_sha256': hashlib.sha256((compiler.parent / 'Microsoft.Dynamics.Nav.CodeAnalysis.dll').read_bytes()).hexdigest(),
           'system_app_sha256': hashlib.sha256((symbols / 'System.app').read_bytes()).hexdigest(),
           'runtime_execution_proof': False}
(root / 'artifacts/al-oracle.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'exit': result.returncode, 'negative_exit': refused.returncode,
                  'success': valid, 'negative_errors': errors}), flush=True)
raise SystemExit(0 if valid else 1)
