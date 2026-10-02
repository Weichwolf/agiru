import importlib.util
import json
from pathlib import Path

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
build = json.loads((artifacts / 'llvm-compiler-ut.json').read_text())
assert build['exit'] == 2 and build['source_unchanged']
assert verify.digest(source) == build['source_before'] == build['source_after']
run = json.loads((source / 'build/ut.log.run.json').read_text())
assert run['build_exit'] == 2 and run['status'] == 2
assert run['infrastructure_errors'] == ['build exited 2; the UT runner was not started']
manifest = json.loads((source / 'build/ut.log.manifest.json').read_text())
previous = json.loads((root.parent / 'chart-20261001/artifacts/ut-milestone.log.manifest.json').read_text())


def identities(entries):
    return {(entry['id'], entry['name'], method)
            for entry in entries for method in entry['methods']}


expected = identities(manifest)
assert expected == identities(previous) and len(manifest) == 80 and len(expected) == 2310
results = [json.loads(line) for line in
           (source / 'build/ut.log.results.jsonl').read_text().splitlines()]
actual = {(row['codeunit_id'], row['codeunit'], row['method']) for row in results}
assert actual == expected and len(results) == len(expected)
assert all(row['status'] == 'missing' and row['passed'] is False for row in results)
log = (artifacts / 'llvm-compiler-ut.log').read_text()
assert log.count('error:') == 1
assert 'PageFieldsSelectionList.cpp:19:17: error:' in log
assert 'return Format(Caption);' in log
receipt = {
    'source_sha256': build['source_after'],
    'source_unchanged': True,
    'build_exit': 2,
    'elapsed_seconds': build['elapsed_seconds'],
    'runner_started': False,
    'codeunits': 80,
    'methods': 2310,
    'executed': 0,
    'passed': 0,
    'missing': len(results),
    'incomplete_codeunits': 80,
    'identity_gains': 0,
    'identity_losses': 0,
    'first_compiler_error': 'PageFieldsSelectionList.cpp:19:17: Format(Caption)',
    'unbound_system_table': {'id': 2000000171, 'name': 'Page Table Field'},
    'G1_proved': False,
}
(artifacts / 'ut-proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
