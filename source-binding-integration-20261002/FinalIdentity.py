import importlib.util
import json
from pathlib import Path

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
tested = json.loads((task / 'artifacts/main-tests.json').read_text())
expected = {'board/README.md',
    'board/0033_app_boundaries_and_extension_merges_will_be_explicit_and_enforced.md',
    'board/0034_every_object_kind_will_have_a_truthful_translation_and_runtime_census.md',
    'board/0038_the_complete_generated_tree_will_compile_and_link_without_slice_fallbacks.md',
    'board/0058_every_ut_run_will_reconcile_results_with_an_independent_source_manifest.md',
    'board/0073_generated_expressions_will_preserve_al_types_and_evaluation_effects.md',
    'board/0589_the_toolchain_will_be_reproducible_and_every_gate_will_fail_reliably.md'}
old = set(verify.files(source))
new = set(verify.files(main))
assert old == new
changed = {str(path) for path in old if (main / path).read_bytes() != (source / path).read_bytes()}
assert changed == expected, changed
assert verify.digest(source) == tested['source_after']
receipt = dict(tested_image_sha256=tested['source_after'], main_image_sha256=verify.digest(main),
    changed_after_testing=sorted(changed), compiler_runtime_gate_and_generated_inputs_unchanged=True,
    goal_complete=False, native_system_source_binding_remaining=True,
    full_lint_remaining=True, G1_proved=False)
(task / 'artifacts/final-identity.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
