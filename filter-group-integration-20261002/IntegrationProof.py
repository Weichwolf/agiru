import importlib.util
import json
from pathlib import Path
import re

task = Path(__file__).resolve().parent
main = task.parent.parent
run = task.parent / 'verify-cache-20261002/source/build/verify/20261002T064256Z-4'
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
result = json.loads((run / 'result.json').read_text())
assert result['status'] == 'failed' and result['target_exits'] == {'all': 2, 'test': 2}
assert result['post_source_sha256'] == result['source_sha256'] == verify.digest(Path(result['source'])) == verify.digest(Path(result['build_source']))
assert result['post_system_symbols_sha256'] == result['system_symbols_sha256'] == verify.digest(run / 'system_symbols')
pattern = re.compile(r'^([\w -]+): (\d+) check\(s\), (\d+) red$', re.M)

def counts(text):
    return {match[1]: (int(match[2]), int(match[3])) for match in pattern.finditer(text)}

log = (run / 'verify.log').read_text()
previous = (task / 'artifacts/main-tests.log').read_text()
assert counts(log) == counts(previous)
assert set(re.findall(r'^FAIL  an exception left ([\w -]+)$', log, re.M)) == set(re.findall(r'^FAIL  an exception left ([\w -]+)$', previous, re.M))
assert 'Ran 140 tests' in log and '\nOK\n' in log and 'test: 97 case(s), 26 red' in log
assert log.count('Connection:') == previous.count('Connection:') == 26
assert 'unity_stable/785/root_cxx.cxx.o' in log and 'PageFieldsSelectionList.cpp:19:17: error:' in log
assert 'return Format(Caption);' in log
assert not re.search(r'missing executable|skipped|unknown exception', log, re.I)
receipt = {'snapshot': str(run), 'result': result, 'snapshot_lane_and_system_hashes_match': True,
    'same_all_local_cpp_counts_and_statuses_as_main': True, 'toolchain_tests': 140,
    'filter_group_checks': 137, 'local_cases': 97, 'database_failures': 26,
    'blocker': 'Missing native Page Table Field SourceTable binding at root 785, PageFieldsSelectionList.cpp:19 Format(Caption).',
    'configured_AL_methods_executed': 0, 'configured_AL_methods_missing': 2310,
    'per_run_cache_hit_comparison_available': False, 'full_tree_or_G1_proved': False}
(task / 'artifacts/integration-result.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key != 'result'}))
