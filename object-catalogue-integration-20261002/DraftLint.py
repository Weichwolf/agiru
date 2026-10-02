import json
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
program = root / 'candidate/ObjectCatalogueGate.cpp'
original = source / 'test/gate/ObjectCatalogueGate.cpp'
entries = json.loads((source / 'compile_commands.json').read_text())
entry = next(row for row in entries if row['file'] == str(original))
entry = {key: value.replace(str(original), str(program)) if isinstance(value, str) else value for key, value in entry.items()}
(program.parent / 'compile_commands.json').write_text(json.dumps([entry], indent=2) + '\n')
command = ['clang-tidy-19', '-p', str(program.parent), '--config-file=' + str(source / '.clang-tidy'), '--quiet',
           '--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang', '--extra-arg=max-nodes=50000',
           '--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang', '--extra-arg=optin.performance.Padding:AllowedPad=64', str(program)]
result = subprocess.run(command, cwd=source, capture_output=True, text=True)
output = result.stdout + result.stderr
(root / 'artifacts/draft-lint.log').write_text(output)
findings = {re.sub(r':\d+:\d+:', ':LOC:', line.replace(str(source), '<source>').replace(str(program), '<source>/test/gate/ObjectCatalogueGate.cpp'))
            for line in output.splitlines() if ': error:' in line or ': warning:' in line}
old = json.loads((root / 'artifacts/targeted-lint.json').read_text())
inherited = set(old['rows'][-1]['inherited_header_findings'])
receipt = {'exit': result.returncode, 'findings': sorted(findings), 'new_findings': sorted(findings - inherited),
           'verified_byte_identical_inherited_headers': sorted(findings & inherited), 'full_lint_passed': False}
(root / 'artifacts/draft-lint.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
assert not receipt['new_findings']
