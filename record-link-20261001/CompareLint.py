from pathlib import Path
import json
import re
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
artifacts = root / 'artifacts'
import sys
unit = sys.argv[1]
label = 'body' if unit == 'src/gen/BodyWriter.cpp' else 'storage'
before = artifacts / ('lint-before-' + label + '.log')
command = ['clang-tidy-19', '-p', str(origin), '--quiet',
           '--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang',
           '--extra-arg=max-nodes=50000', '--extra-arg=-Xclang',
           '--extra-arg=-analyzer-config', '--extra-arg=-Xclang',
           '--extra-arg=optin.performance.Padding:AllowedPad=64', str(origin / unit)]
with before.open('w') as log:
    checked = subprocess.run(command, cwd=origin, stdout=log, stderr=subprocess.STDOUT,
                             timeout=120)
after = artifacts / ('lint-' + label + '-targeted.log')
def findings(path, owner):
    pattern = r'^(.+?):\d+:\d+: (?:warning|error): (.+?)(?: \[([^\]]+)\])?$'
    return sorted((file.removeprefix(str(owner) + '/'), message, check or '')
                  for file, message, check in re.findall(pattern, path.read_text(), re.M))
old, new = findings(before, origin), findings(after, source)
added, removed = list(new), list(old)
for item in old:
    if item in added:
        added.remove(item)
        removed.remove(item)
receipt = {'unit': unit, 'before_command': command, 'before_exit': checked.returncode,
           'before_findings': len(old), 'after_findings': len(new),
           'added': added, 'removed': removed, 'baseline_raised': False, 'full_lint_green': False}
(artifacts / ('lint-' + label + '-comparison.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
raise SystemExit(0 if not added else 1)
