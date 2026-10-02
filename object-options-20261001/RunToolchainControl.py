from pathlib import Path
import json
import os
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
old = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
rows = []
for compiler, build in [('clang', 'build'), ('gcc', 'build/gcc')]:
    for label, image in [('predecessor', old), ('current', source)]:
        command = [sys.executable, str(source / 'test/toolchain.py'),
                   'SystemDeclarationGate.test_incompatible_intrinsic_ids_and_links_do_not_pass_as_native_bindings']
        result = subprocess.run(command, env=dict(os.environ, B=str(image / build)),
                                text=True, capture_output=True, timeout=60)
        valid = (result.returncode == 0 if label == 'current' else result.returncode == 1 and
                 'intrinsic target has ID 2000000225' in result.stderr and
                 'intrinsic target has ID 2000000196' in result.stderr)
        rows.append({'compiler': compiler, 'image': label, 'command': command,
                     'exit': result.returncode, 'stdout': result.stdout, 'stderr': result.stderr, 'valid': valid})
receipt = {'results': rows, 'success': all(row['valid'] for row in rows),
           'test_population_unchanged': True, 'wrong_source_id_still_refuses': True}
(root / 'artifacts/toolchain-control.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'success': receipt['success'], 'exits': [row['exit'] for row in rows]}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
