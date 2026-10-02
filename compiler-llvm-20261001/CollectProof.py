import ast
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
main = root.parent.parent
parent = root.parent / 'chart-20261001/source'
artifacts = root / 'artifacts'


def population(path):
    tree = ast.parse(path.read_text())
    return {(node.name, method.name) for node in tree.body if isinstance(node, ast.ClassDef)
            for method in node.body if isinstance(method, ast.FunctionDef)
            and method.name.startswith('test_')}


assert population(source / 'test/toolchain.py') == (
    population(main / 'test/toolchain.py') | population(parent / 'test/toolchain.py'))
assert len(population(source / 'test/toolchain.py')) == 147
tests = json.loads((artifacts / 'llvm-compiler-post-generation-tests.json').read_text())
assert tests['exit'] == 2 and tests['source_unchanged']
log = (artifacts / 'llvm-compiler-post-generation-tests.log').read_text()
for text in ('Ran 147 tests', '\nOK\n', 'test: 93 case(s), 26 red',
             'DataMeasure: 49 check(s), 0 red', 'DataTable: 38 check(s), 0 red',
             'NativeObject: 1365 check(s), 0 red', 'GenTableBinding: 169 check(s), 0 red'):
    assert text in log, text
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
assert verify.digest(source) == tests['source_after']
assert verify.digest(parent) == '7d8acdccca3fa0f2a9b031199ac6c4522915d5f686b29de15e638c47a0098cb9'
inputs = json.loads((artifacts / 'inputs.json').read_text())
assert verify.digest(root / 'bc_source') == inputs['bc_source_sha256']
assert verify.digest(root / 'system_symbols') == inputs['system_symbols_sha256']
current = json.loads((source / 'build/ut.log.manifest.json').read_text())
previous = json.loads((main / 'build/chart-20261001/artifacts/ut-milestone.log.manifest.json').read_text())


def identities(entries):
    return {(item['id'], item['name'], method) for item in entries for method in item['methods']}


assert identities(current) == identities(previous)
assert len(current) == 80 and len(identities(current)) == 2310
generation = json.loads((artifacts / 'generation.json').read_text())
assert generation['exit'] == 0 and not generation['missing_slice_sources']
assert generation['source_after'] == tests['source_after']
files = {}
for name in ('include/dotnet/BusinessChart.h', 'test/gate/DataMeasureGate.cpp', 'test/gate/DataTableGate.cpp'):
    value = (source / name).read_bytes()
    assert value == (main / name).read_bytes(), name
    files[name] = hashlib.sha256(value).hexdigest()
gate_results = json.loads((artifacts / 'main-gates.json').read_text())
assert len(gate_results) == 2 and all(row['exit'] == 0 for row in gate_results)
controls = json.loads((artifacts / 'numeric-controls.json').read_text())
assert [row['exit'] for row in controls] == [1, 0, 0]
declaration = (root / 'bc_source/System Application/App/Business Chart/src/BusinessChartType.Enum.al').read_text()
declared = {name.strip('"'): int(value) for value, name in re.findall(
    r'value\((\d+);\s*("[^"]+"|\w+)\)', declaration)}
header = (main / 'include/dotnet/BusinessChart.h').read_text()
kind = re.search(r'enum class Kind : Integer \{(.*?)\n  \};', header, re.S)[1]
actual = {name: int(value) for name, value in re.findall(r'(\w+)\s*=\s*(\d+)', kind)}
assert actual == declared and len(actual) == 15
deps = {}
for target in ('libagiru_al.so', 'libagiru_net.so', 'libagiru_db.so', 'libagiru_gen.so', 'libagiru_rt.so', 'agirutc'):
    result = subprocess.run(['readelf', '-d', str(source / 'build' / target)], capture_output=True, text=True, check=True)
    names = re.findall(r'Shared library: \[([^]]+)\]', result.stdout)
    assert 'libc++.so.1' in names and not any(name.startswith(('libstdc++', 'libgcc_s')) for name in names)
    deps[target] = names
consumer = json.loads((artifacts / 'actual-chart.json').read_text())
assert consumer['source_unchanged'] and consumer['compile_exit'] != 0 and consumer['exit'] is None
assert 'undefined symbol:' in consumer['diagnostics']
receipt = {'source_sha256': tests['source_after'], 'prototype_parent_unchanged': True,
           'verified_main_files': files, 'declaration_members': actual,
           'main_gates': gate_results, 'toolchain_tests_green': 147, 'toolchain_skipped': 0,
           'local_cases': 93, 'local_red': 26, 'old_numeric_checks_red': 5,
           'ut_codeunits': 80, 'ut_methods': 2310, 'ut_identity_gains': 0, 'ut_identity_losses': 0,
           'generation': generation, 'frozen_inputs': inputs, 'llvm_dependencies': deps,
           'actual_chart_checks_missing': 12, 'actual_chart_dependency_link_open': True,
           'compiler_chain_production_integrated': False, 'full_tree_or_G1_proved': False}
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: receipt[key] for key in ('source_sha256', 'toolchain_tests_green',
                  'local_cases', 'local_red', 'ut_methods', 'actual_chart_checks_missing')}))
