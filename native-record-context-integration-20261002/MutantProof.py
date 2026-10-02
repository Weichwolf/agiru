import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
source = task / 'source'
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
original = (source / 'src/gen/BodyWriter.cpp').read_text()
needle = 'return DoorCalls(member.field) && RecordFieldOf(member).empty();'
assert original.count(needle) == 1
mutant = artifacts / 'NormalizedFieldMutant.cpp'
assert mutant.read_text() == original.replace(needle, 'return DoorCalls(member.field) && !HasField(member);')
before = verify.digest(source)
result = subprocess.run([str(artifacts / 'NormalizedFieldMutantGate')], cwd=task,
                        capture_output=True, text=True)
(artifacts / 'normalized-field-mutant.log').write_text(result.stdout + result.stderr)
receipt = {'source_sha256': before, 'source_unchanged': verify.digest(source) == before,
    'mutant_source_sha256': hashlib.sha256(mutant.read_bytes()).hexdigest(),
    'exit': result.returncode, 'checks': 159, 'red': 1,
    'defect': 'Normalized quoted field identity captures a distinct TableCaption property method.',
    'runtime_and_public_headers_unchanged': True}
(artifacts / 'normalized-field-mutant.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert receipt['source_unchanged'] and result.returncode == 1
assert 'GenSourceBinding: 159 check(s), 1 red' in result.stdout
assert 'a quoted field does not capture the TableCaption method' in result.stdout
