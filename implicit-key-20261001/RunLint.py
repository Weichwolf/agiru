from pathlib import Path
import json
import os
import shutil
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
receipt_path = artifacts / 'lint-commands.json'
receipts = json.loads(receipt_path.read_text()) if receipt_path.exists() else []
for stem, unit in (('parser', 'src/al/Parser.cpp'), ('table-writer', 'src/gen/TableWriter.cpp'),
                   ('tc-main', 'src/tc/Main.cpp'), ('key-gate', 'test/gate/GenKeyGate.cpp')):
    if len(sys.argv) > 1 and stem not in sys.argv[1:]:
        continue
    command = ['make', 'lint-one', f'UNIT={unit}', 'JOBS=2']
    result = subprocess.run(command, cwd=source, text=True, capture_output=True,
                            env=dict(os.environ, CCACHE_DIR='/tmp/agiru-native-20261001.WC8RiY/ccache'),
                            timeout=180)
    (artifacts / f'{stem}-lint-command.log').write_text(result.stdout + result.stderr)
    shutil.copy2(source / 'build/lint/targeted.log', artifacts / f'{stem}-targeted.log')
    shutil.copy2(source / 'build/lint/targeted-units.json', artifacts / f'{stem}-targeted-units.json')
    receipts.append({'unit': unit, 'exit': result.returncode})
    print(unit, result.returncode, flush=True)
receipt_path.write_text(json.dumps(receipts, indent=2) + '\n')
