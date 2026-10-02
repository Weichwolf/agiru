import hashlib
import importlib.util
import json
from pathlib import Path

task = Path(__file__).resolve().parent
root = task.parent.parent
own = task / 'source'
baseline = task.parent / 'source-binding-integration-20261002/source'
spec = importlib.util.spec_from_file_location('verify_snapshot', root / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)

def files(tree):
    return {str(path): hashlib.sha256((tree / path).read_bytes()).hexdigest()
            for path in verify.files(tree) if path.name != 'compile_commands.json'}

current, previous = files(root), files(baseline)
changed = sorted(path for path in current.keys() & previous.keys() if current[path] != previous[path])
new, removed = sorted(current.keys() - previous.keys()), sorted(previous.keys() - current.keys())
assert not new and not removed
assert all(path.startswith('board/') or path in ('scripts/fetch_symbols.py', 'test/toolchain.py') for path in changed)
initial_generation = json.loads((task / 'artifacts/generation.json').read_text())
generated = {str(path.relative_to(own / 'apps')): hashlib.sha256(path.read_bytes()).hexdigest()
             for path in (own / 'apps').rglob('*') if path.is_file()}
repeat = json.loads((task / 'artifacts/repeat-generation.json').read_text())
assert generated == initial_generation['generated_after_hashes']
assert repeat['source_before'] == repeat['source_after'] == verify.digest(own)
receipt = dict(main_source_sha256=verify.digest(root), own_source_sha256=verify.digest(own),
    main_changed_since_previous_tested_source=changed, main_added=new, main_removed=removed,
    main_cpp_runtime_and_generated_bytes_unchanged=True,
    own_repeat_generated_files=len(generated), own_repeat_all_bytes_identical=True,
    own_repeat_rewritten_mtimes=repeat['changed'], own_repeat_mtime_gate_passed=False,
    only_package_verifier_promoted=True, native_binding_promoted=False, G1_proved=False)
(task / 'artifacts/final-identity.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
