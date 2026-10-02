import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
artifacts = task / 'artifacts'
bc = task.parent / 'compiler-llvm-20261001/bc_source'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
inputs = json.loads((task.parent / 'compiler-llvm-20261001/artifacts/inputs.json').read_text())
assert verify.digest(bc) == inputs['bc_source_sha256']
identities = {str(tree): verify.digest(tree) for tree in (main, source)}
rows = []
for label, tree in [('before', main), ('after', source)]:
    output = artifacts / ('census-' + label + '.json')
    command = ['python3', str(tree / 'scripts/scope_inventory.py'), str(bc), '--output', str(output)]
    result = subprocess.run(command, cwd=tree, capture_output=True, text=True)
    (artifacts / ('census-' + label + '.log')).write_text(result.stdout + result.stderr)
    rows.append({'label': label, 'exit': result.returncode,
                 'report_sha256': hashlib.sha256(output.read_bytes()).hexdigest()})
report = json.loads((artifacts / 'census-after.json').read_text())
receipt = {'source_hashes': identities, 'rows': rows,
    'sources_unchanged': all(verify.digest(Path(tree)) == value for tree, value in identities.items()),
    'complete_inventory_bytes_equal': rows[0]['report_sha256'] == rows[1]['report_sha256'],
    'summary': report['summary'], 'errors': report['errors'],
    'bc_revision': inputs['bc_revision'], 'bc_input_sha256': inputs['bc_source_sha256'],
    'raw_population_completed': False, 'G1_proved': False}
(artifacts / 'census-comparison.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert receipt['sources_unchanged'] and receipt['complete_inventory_bytes_equal']
assert report['summary']['files'] == 36697 and report['summary']['test_methods'] == 112055
assert report['summary']['product_excluded_test_methods'] == 15
assert report['summary']['product_required_test_methods'] == 112040
assert report['summary']['unmeasured_files'] == 1 and rows[0]['exit'] == rows[1]['exit'] == 1
assert verify.digest(bc) == inputs['bc_source_sha256']
