from pathlib import Path
import json
import os
import shutil
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
receipts = []
for stem, unit in (('body-writer', 'src/gen/BodyWriter.cpp'), ('text-gate', 'test/gate/TextGate.cpp')):
    command = ['make', 'lint-one', f'UNIT={unit}', 'JOBS=2']
    result = subprocess.run(command, cwd=source, text=True, capture_output=True,
                            env=dict(os.environ, CCACHE_DIR='/tmp/agiru-native-20261001.WC8RiY/ccache'),
                            timeout=120)
    (artifacts / f'{stem}-lint-command.log').write_text(result.stdout + result.stderr)
    shutil.copy2(source / 'build/lint/targeted.log', artifacts / f'{stem}-targeted.log')
    shutil.copy2(source / 'build/lint/targeted-units.json', artifacts / f'{stem}-targeted-units.json')
    receipts.append({'unit': unit, 'exit': result.returncode})
    print(unit, result.returncode, flush=True)
(artifacts / 'lint-commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
