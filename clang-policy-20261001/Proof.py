import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import unittest

root = Path(__file__).resolve().parents[2]
output = Path(__file__).resolve().parent
parent = root / 'build/chart-20261001/source'
paths = ('Makefile', 'CMakeLists.txt', 'scripts/install.sh', 'scripts/first_gap.sh',
         'scripts/tree_syntax.sh', 'scripts/compile_cost.sh')


def hashes(folder):
    return {name: hashlib.sha256((folder / name).read_bytes()).hexdigest() for name in paths}


spec = importlib.util.spec_from_file_location('toolchain_policy', root / 'test/toolchain.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
before = hashes(parent)
results = {}
for name, source in (('current', root), ('unchanged-parent', parent)):
    module.NativeToolchainGate.root = source
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(module.NativeToolchainGate)
    with (output / f'{name}.log').open('w') as log:
        result = unittest.TextTestRunner(stream=log, verbosity=2).run(suite)
    results[name] = {'tests': result.testsRun, 'failures': len(result.failures),
                     'errors': len(result.errors), 'skipped': len(result.skipped),
                     'failed_tests': sorted({test.id().split(' ', 1)[0]
                                             for test, _ in result.failures}),
                     'source': str(source), 'input_sha256': hashes(source)}
assert hashes(parent) == before, 'live parent inputs changed'
assert results['current']['tests'] == results['unchanged-parent']['tests'] == 5
assert results['current']['failures'] == results['current']['errors'] == 0
assert results['unchanged-parent']['failures'] == 7
assert len(results['unchanged-parent']['failed_tests']) == 5
assert results['unchanged-parent']['errors'] == 0
assert all(result['skipped'] == 0 for result in results.values())
checks = (
    ['python3', 'test/toolchain.py',
     'CompilerCacheGate.test_targeted_lint_refreshes_the_graph_without_building_objects',
     'CompilerCacheGate.test_single_gate_preserves_build_and_test_failures',
     'CompilerCacheGate.test_compiler_names_resolve_through_path_and_mismatches_still_fail', '-v'],
    ['sh', '-n', 'scripts/install.sh'], ['sh', '-n', 'scripts/tree_syntax.sh'],
    ['sh', '-n', 'scripts/first_gap.sh'], ['sh', '-n', 'scripts/compile_cost.sh'],
    ['git', 'diff', '--check'])
for index, command in enumerate(checks):
    with (output / f'check-{index}.log').open('w') as log:
        result = subprocess.run(command, cwd=root, stdout=log, stderr=subprocess.STDOUT, timeout=30)
    results[f'check-{index}'] = {'command': command, 'exit': result.returncode}
    assert result.returncode == 0
results['scope'] = 'Five LLVM compiler/library policy controls plus three existing Make failure/ownership controls; full build/test receipts are separate.'
results['live_parent_inputs_unchanged'] = True
(output / 'proof.json').write_text(json.dumps(results, indent=2) + '\n')
print(json.dumps(results, indent=2))
