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
if unit != 'src/net/Guid.cpp':
    raise SystemExit('expected src/net/Guid.cpp')
label = 'guid'
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
metric_reductions = []
complexity = re.compile(r"^(function '.+' has cognitive complexity of )(\d+)( \(threshold \d+\))$")
for previous in list(removed):
    before_metric = complexity.fullmatch(previous[1])
    if before_metric is None:
        continue
    for current in list(added):
        after_metric = complexity.fullmatch(current[1])
        if (after_metric is not None and previous[0] == current[0] and previous[2] == current[2]
                and before_metric[1] == after_metric[1] and before_metric[3] == after_metric[3]
                and int(after_metric[2]) < int(before_metric[2])):
            added.remove(current)
            removed.remove(previous)
            metric_reductions.append({'before': previous, 'after': current})
            break
receipt = {'unit': unit, 'before_command': command, 'before_exit': checked.returncode,
           'before_findings': len(old), 'after_findings': len(new),
           'added': added, 'removed': removed, 'metric_reductions': metric_reductions,
           'baseline_raised': False, 'full_lint_green': False}
(artifacts / ('lint-' + label + '-comparison.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
raise SystemExit(0 if not added else 1)
