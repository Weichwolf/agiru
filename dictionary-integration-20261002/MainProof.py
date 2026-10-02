import ast
import importlib.util
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
main = root.parent.parent
artifacts = root / 'artifacts'
frozen = root.parent / 'verify-cache-20261002/source'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
tested = json.loads((artifacts / 'main-post-cache-tests.json').read_text())
assert tested['exit'] == 2 and tested['source_before'] == tested['source_after'] == verify.digest(main)
log = (artifacts / 'main-post-cache-tests.log').read_text()
assert 'test: 93 case(s), 26 red' in log and 'Ran 136 tests' in log and '\nOK\n' in log
for name, count in [('Dictionary', 70), ('DesignerConstants', 45), ('GenReceiver', 10)]:
    assert f'{name}: {count} check(s), 0 red' in log


def test_ids(path):
    module = ast.parse(path.read_text())
    return {node.name + '.' + method.name for node in module.body if isinstance(node, ast.ClassDef)
            for method in node.body if isinstance(method, ast.FunctionDef) and method.name.startswith('test_')}


previous = test_ids(root / 'source/test/toolchain.py')
current = test_ids(main / 'test/toolchain.py')
assert len(previous) == 135 and len(current) == 136
assert previous <= current and current - previous == {'SnapshotGate.test_deleted_tracked_caches_do_not_abort_freezing'}
assert sum(name.startswith('SourceRevisionGate.') for name in current) == 5
paths = set(verify.files(main)) | set(verify.files(frozen))
differences = sorted(str(path) for path in paths
                     if not (main / path).is_file() or not (frozen / path).is_file()
                     or (main / path).read_bytes() != (frozen / path).read_bytes())
assert differences and all(path.startswith('board/') for path in differences), differences
dependencies = []
for name in ['agirutc', 'libagiru_al.so', 'libagiru_gen.so', 'libagiru_net.so',
             'libagiru_db.so', 'libagiru_rt.so', 'gate_DictionaryGate']:
    result = subprocess.run(['readelf', '-d', str(main / 'build' / name)], capture_output=True, text=True)
    assert result.returncode == 0 and 'libstdc++' not in result.stdout and 'libgcc_s' not in result.stdout
    dependencies.append({'file': name, 'dynamic_section': result.stdout})
receipt = {'main_test_source_sha256': tested['source_after'], 'local_cases': 93, 'local_failed': 26,
           'python_tests': 136, 'skipped': 0, 'previous_python_tests': 135,
           'new_test_identities': sorted(current - previous), 'lost_test_identities': [],
           'llvm_ELF_checks': dependencies,
           'main_vs_frozen_differences': differences, 'only_board_differs_from_frozen': True,
           'compiler_runtime_generator_and_generated_inputs_match_frozen': True,
           'frozen_origin_sha256': verify.digest(frozen),
           'snapshot_cache_control': 'snapshot-cache-{old,new}.json',
           'full_lint_passed': False, 'integration_or_G1_passed': False}
(artifacts / 'main-proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: receipt[key] for key in ('main_test_source_sha256', 'python_tests',
                                              'local_cases', 'local_failed', 'lost_test_identities',
                                              'only_board_differs_from_frozen')}))
