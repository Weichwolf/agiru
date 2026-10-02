import hashlib
import importlib.util
import json
from pathlib import Path

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)

def files(tree):
    return {str(relative): hashlib.sha256((tree / relative).read_bytes()).hexdigest() for relative in verify.files(tree)}

current, reviewed = files(main), files(source)
changed = sorted(path for path in current.keys() & reviewed.keys() if current[path] != reviewed[path])
assert current.keys() == reviewed.keys()
expected = {'board/README.md',
    'board/0038_the_complete_generated_tree_will_compile_and_link_without_slice_fallbacks.md',
    'board/0044_record_operations_will_share_one_correct_sql_and_temporary_contract.md',
    'board/0058_every_ut_run_will_reconcile_results_with_an_independent_source_manifest.md',
    'board/0073_generated_expressions_will_preserve_al_types_and_evaluation_effects.md'}
assert set(changed) == expected
proof = json.loads((task / 'artifacts/main-proof.json').read_text())
assert verify.digest(source) == proof['source_sha256']
assert all(hashlib.sha256((main / 'build' / entry['name']).read_bytes()).hexdigest() == entry['sha256'] for entry in proof['elf'])
receipt = {'main_source_sha256': verify.digest(main), 'tested_source_sha256': proof['source_sha256'],
    'changes_since_reviewed_image': changed, 'only_board_changed_after_verification': True,
    'all_cpp_runtime_generator_test_and_generated_inputs_match_reviewed_image': True,
    'no_source_or_generated_path_loss': True, 'goal_status': 'active', 'G1_proved': False}
(task / 'artifacts/final-identity.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
