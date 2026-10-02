from pathlib import Path
import hashlib
import json
import subprocess

root = Path(__file__).resolve().parent
project = root / 'artifacts/al-oracle'
project.mkdir(parents=True, exist_ok=True)
(project / 'app.json').write_text(json.dumps({
    'id': 'a2b15043-9dac-4b2d-a3f1-acd7e9271d38', 'name': 'Object Options Oracle',
    'publisher': 'agiru', 'version': '1.0.0.0', 'runtime': '17.0',
    'platform': '28.0.0.0', 'idRanges': [{'from': 50231, 'to': 50232}]}))
for name in ('StoredFlag.Table.al', 'SavedOptions.Codeunit.al'):
    (project / name).write_bytes((root / name).read_bytes())
compiler = root.parent / 'text-guid-20261001/tools/al/alc'
symbols = Path('/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH')
command = [str(compiler), '/project:' + str(project), '/packagecachepath:' + str(symbols),
           '/out:' + str(project / 'options.app'), '/warnaserror+', '/parallel-']
result = subprocess.run(command, text=True, capture_output=True, timeout=60)
receipt = {'command': command, 'exit': result.returncode, 'stdout': result.stdout,
           'stderr': result.stderr, 'success': result.returncode == 0,
           'compiler_assembly_sha256': hashlib.sha256((compiler.parent / 'Microsoft.Dynamics.Nav.CodeAnalysis.dll').read_bytes()).hexdigest(),
           'system_app_sha256': hashlib.sha256((symbols / 'System.app').read_bytes()).hexdigest(),
           'runtime_execution_proof': False}
(root / 'artifacts/al-oracle.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'exit': result.returncode, 'stdout': result.stdout, 'stderr': result.stderr}), flush=True)
raise SystemExit(result.returncode)
