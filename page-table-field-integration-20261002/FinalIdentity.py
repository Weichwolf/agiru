import importlib.util
import json
from pathlib import Path

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
different = []
for relative in set(verify.files(main)) | set(verify.files(source)):
    if (main / relative).read_bytes() != (source / relative).read_bytes():
        different.append(str(relative))
assert different and all(name.startswith('board/') for name in different), different
tests = json.loads((task / 'artifacts/main-tests.json').read_text())
assert verify.digest(source) == tests['source_after'] and tests['source_unchanged']
receipt = {'current_main_source_sha256': verify.digest(main),
           'tested_main_source_sha256': tests['source_after'],
           'post_test_board_only_changes': sorted(different),
           'compiler_runtime_headers_gates_and_generated_inputs_equal_tested_image': True,
           'production_promoted': True, 'G1_achieved': False,
           'next_action': 'Implement common source-declaration binding and explicit System input consumption, then regenerate/rebuild the full selected tree.'}
(task / 'artifacts/final-identity.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
