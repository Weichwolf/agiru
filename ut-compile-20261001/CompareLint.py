from pathlib import Path
import json
import re
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
artifacts = root / 'artifacts'
rows = []
for label, unit in (('field', 'src/rt/written/PlatformField.cpp'),
                    ('codeunit', 'src/gen/CodeunitWriter.cpp'),
                    ('table', 'src/gen/TableWriter.cpp')):
    before = artifacts / f'lint-before-{label}.log'
    command = ['clang-tidy-19', '-p', str(origin), '--quiet',
               '--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang',
               '--extra-arg=max-nodes=50000', '--extra-arg=-Xclang',
               '--extra-arg=-analyzer-config', '--extra-arg=-Xclang',
               '--extra-arg=optin.performance.Padding:AllowedPad=64', str(origin / unit)]
    with before.open('w') as log:
        checked = subprocess.run(command, cwd=origin, stdout=log, stderr=subprocess.STDOUT,
                                 timeout=120)
    after = artifacts / f'lint-{label}-targeted.log'
    def findings(path, owner):
        pattern = r'^(.+?):\d+:\d+: (?:warning|error): (.+?)(?: \[([^\]]+)\])?$'
        return sorted((file.removeprefix(str(owner) + '/'), message, check or '')
                      for file, message, check in re.findall(pattern, path.read_text(), re.M))
    old, new = findings(before, origin), findings(after, source)
    added = list(new)
    removed = list(old)
    for item in old:
        if item in added:
            added.remove(item)
            removed.remove(item)
    rows.append({'unit': unit, 'before_command': command, 'before_exit': checked.returncode,
                 'before_findings': len(old), 'after_findings': len(new),
                 'added': added, 'removed': removed})
receipt = {'units': rows, 'baseline_raised': False, 'full_lint_green': False}
(artifacts / 'lint-comparison.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
raise SystemExit(0 if not any(row['added'] for row in rows) else 1)
