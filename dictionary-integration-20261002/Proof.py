import ast
import hashlib
import importlib.util
import json
from pathlib import Path

root = Path(__file__).resolve().parent
main = root.parent.parent
source = root / 'source'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identity = verify.digest(source)
assert identity == '0581bb5dbba3e123038eb63c4d250d514c92bcc86a96e242a16ca0e5636b383c'
assert verify.digest(main) == identity, 'main and validated source image differ'
generation = json.loads((artifacts / 'main-generation.json').read_text())
assert generation['exit'] == 0 and generation['source_after'] == identity
tests = json.loads((artifacts / 'post-generation-tests.json').read_text())
assert tests['source_before'] == tests['source_after'] == identity and tests['exit'] == 2
log = (artifacts / 'post-generation-tests.log').read_text()
assert 'test: 93 case(s), 26 red' in log and 'Ran 135 tests' in log and '\nOK\n' in log
for gate, count in [('Dictionary', 70), ('DesignerConstants', 45), ('GenReceiver', 10)]:
    assert f'{gate}: {count} check(s), 0 red' in log
module = ast.parse((source / 'test/toolchain.py').read_text())
identities = sorted(node.name + '.' + method.name for node in module.body
                    if isinstance(node, ast.ClassDef) for method in node.body
                    if isinstance(method, ast.FunctionDef) and method.name.startswith('test_'))
assert len(identities) == 135 and sum(name.startswith('SourceRevisionGate.') for name in identities) == 5
assert (main / 'test/toolchain.py').read_bytes() == (source / 'test/toolchain.py').read_bytes()
lint = json.loads((artifacts / 'targeted-lint.json').read_text())
assert lint['sources_unchanged'] and not any(row['new_findings'] for row in lint['rows'])
full_lint = json.loads((source / 'build/lint/units.json').read_text())
assert full_lint['available'] == full_lint['checked'] == 193 and full_lint['failed'] == 165
controls = json.loads((artifacts / 'controls.json').read_text())
assert [(row['label'], row['exit']) for row in controls] == [('current', 0), ('old-receiver', 1), ('old-includes', 1)]
for name in ('body-comparison', 'consumer-comparison'):
    comparison = json.loads((artifacts / (name + '.json')).read_text())
    assert not comparison['compile_losses']
prototype = root.parent / 'dictionary-20261001/source'
inherited_controls = []
for unit in ('include/dotnet/DesignerFieldProperty.h', 'include/dotnet/DesignerFieldType.h',
             'test/gate/DesignerConstantsGate.cpp', 'test/gate/DictionaryGate.cpp', 'src/net/Generic.cpp'):
    assert (source / unit).read_bytes() == (prototype / unit).read_bytes()
    inherited_controls.append({'source': unit, 'sha256': hashlib.sha256((source / unit).read_bytes()).hexdigest()})
receipt = {'main_code_and_generated_outputs_promoted': True, 'validated_source_sha256': identity,
           'origin': json.loads((artifacts / 'source-identity.json').read_text()),
           'main_generation': generation, 'post_generation_tests': tests,
           'python_test_identities': identities, 'python_tests': 135, 'skipped': 0,
           'local_cases': 93, 'local_failed': 26, 'gates': {'Dictionary': 70, 'DesignerConstants': 45, 'GenReceiver': 10},
           'changed_body_cases': 13, 'actual_consumer_cases': 8, 'compile_losses': [],
           'generated_files': 24346, 'active_slice_sources': 14212, 'raw_ut_codeunits': 80, 'raw_ut_methods': 2310,
           'inherited_collection_designer_control_sources': inherited_controls,
           'inherited_control_receipt': 'build/dictionary-20261001/artifacts/proof.json',
           'fresh_receiver_and_include_controls': 'controls.json',
           'full_lint': full_lint, 'new_targeted_findings': 0,
           'full_lint_passed': False, 'integration_passed': False,
           'SQL_workflows_or_complete_tree_or_G1_proved': False}
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: receipt[key] for key in ('validated_source_sha256', 'python_tests', 'local_cases',
                                              'local_failed', 'compile_losses', 'generated_files', 'raw_ut_methods')}))
