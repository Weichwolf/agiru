import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
rows = []
for label in ('Before', 'After'):
    command = ['clang-tidy-19', '--quiet', '--checks=-*,clang-analyzer-core.uninitialized.*',
               '--warnings-as-errors=*', str(task / ('Primitive' + label + '.cpp')),
               '--', '-std=c++23', '-stdlib=libc++', '-I' + str(task / 'source/include')]
    result = subprocess.run(command, capture_output=True, text=True)
    rows.append({'label': label, 'command': command, 'exit': result.returncode,
                 'stdout': result.stdout, 'stderr': result.stderr})
receipt = {'rows': rows, 'negative_control_proved': rows[0]['exit'] != 0 and rows[1]['exit'] == 0}
(task / 'artifacts/primitive-control.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'negative_control_proved': receipt['negative_control_proved']}))
