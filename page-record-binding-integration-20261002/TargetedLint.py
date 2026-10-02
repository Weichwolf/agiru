import importlib.util
import json
from pathlib import Path
import re
import subprocess

task = Path(__file__).resolve().parent
after = task / 'source'
before = task.parent.parent
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', after / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identities = {str(tree): verify.digest(tree) for tree in (before, after)}
database = task / 'before-lint'
database.mkdir(exist_ok=True)
entries = json.loads((after / 'compile_commands.json').read_text())
mapped = json.loads(json.dumps(entries).replace(str(after), str(before)))
(database / 'compile_commands.json').write_text(json.dumps(mapped, indent=2) + '\n')
units = ['src/gen/BodyWriter.cpp', 'src/gen/CodeunitWriter.cpp', 'test/gate/GenSourceBindingGate.cpp']
rows = []
inherited = set()
for unit in units:
    comparisons = {}
    for label, tree, commands in [('before', before, database), ('after', after, after)]:
        command = ['clang-tidy-19', '-p', str(commands), '--quiet',
                   '--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang',
                   '--extra-arg=max-nodes=50000',
                   '--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang',
                   '--extra-arg=optin.performance.Padding:AllowedPad=64', str(tree / unit)]
        result = subprocess.run(command, cwd=tree, capture_output=True, text=True)
        output = result.stdout + result.stderr
        (artifacts / ('lint-' + label + '-' + Path(unit).stem + '.log')).write_text(output)
        findings = {re.sub(r':\d+:\d+:', ':LOC:', line.replace(str(tree), '<source>'))
                    for line in output.splitlines() if ': error:' in line or ': warning:' in line}
        comparisons[label] = {'exit': result.returncode, 'findings': sorted(findings)}
        if label == 'before':
            for line in findings:
                match = re.match(r'<source>/(.+\.h):LOC:', line)
                if match and (before / match[1]).read_bytes() == (after / match[1]).read_bytes():
                    inherited.add(line)
    old = set(comparisons['before']['findings'])
    new = set(comparisons['after']['findings'])
    inherited_here = new.intersection(inherited) if unit.startswith('test/') else set()
    rows.append({'unit': unit, 'results': comparisons,
                 'inherited_header_findings': sorted(inherited_here),
                 'new_findings': sorted(new - old - inherited_here), 'removed_findings': sorted(old - new)})
    print(json.dumps({'unit': unit, 'before': len(old), 'after': len(new), 'new': sorted(new - old - inherited_here)}), flush=True)
receipt = {'source_hashes': identities,
           'sources_unchanged': all(verify.digest(Path(tree)) == value for tree, value in identities.items()),
           'rows': rows, 'full_lint_passed': False}
(artifacts / 'targeted-lint-current.json').write_text(json.dumps(receipt, indent=2) + '\n')
assert receipt['sources_unchanged'] and not any(row['new_findings'] for row in rows)
