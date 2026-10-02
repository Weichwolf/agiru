import ast
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import subprocess
import unittest

root = Path(__file__).resolve().parents[2]
artifacts = Path(__file__).resolve().parent
parent = root / 'build/page-table-field-20261001/source'
bc = root / 'build/compiler-llvm-20261001/bc_source'


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


old = module('previous_manifest', parent / 'scripts/ut_manifest.py')
new = module('current_manifest', root / 'scripts/ut_manifest.py')
tests = module('toolchain', root / 'test/toolchain.py')
names = ('test_escaped_boundary_quotes_remain_part_of_object_and_method_names',
         'test_literal_trailing_quote_cannot_invent_a_ut_suffix',
         'test_escaped_quotes_do_not_collapse_distinct_method_identities')
controls = []
for label, scanner in (('previous', old), ('current', new)):
    suite = unittest.TestSuite()
    for name in names:
        case = tests.ManifestGate(name)
        setup = case.setUp

        def replacement(case=case, setup=setup, scanner=scanner):
            setup()
            case.manifest = scanner

        case.setUp = replacement
        suite.addTest(case)
    log = io.StringIO()
    result = unittest.TextTestRunner(stream=log, verbosity=2).run(suite)
    (artifacts / (label + '-controls.log')).write_text(log.getvalue())
    controls.append({'implementation': label, 'tests': result.testsRun,
                     'failures': len(result.failures), 'errors': len(result.errors),
                     'skipped': len(result.skipped)})
assert [(row['tests'], row['failures'], row['errors'], row['skipped'])
        for row in controls] == [(3, 2, 1, 0), (3, 0, 0, 0)]

for name in names:
    case = tests.ManifestGate(name)
    case.setUp()
    try:
        getattr(case, name)()
        text = (case.root / 'Example.al').read_text()
        _, independent = tests.scope_inventory.declarations(text)
        expected = [(row['id'], row['name'], row['methods']) for row in independent
                    if row['kind'] == 'codeunit' and row['test_subtype']
                    and row['name'].endswith((' UT', '-UT', '.UT'))]
        observed = [(row['id'], row['name'], row['methods']) for row in new.scan(case.root)]
        assert expected == observed, (expected, observed)
    finally:
        case.doCleanups()

al = bc / 'Layers/W1/Tests'
before, after = old.scan(al), new.scan(al)
assert before == after and len(after) == 80
assert sum(len(row['methods']) for row in after) == 2310
(artifacts / 'manifest.json').write_text(json.dumps(after, indent=2) + '\n')
paths = ('scripts/ut_manifest.py', 'test/toolchain.py')
hashes = {path: hashlib.sha256((root / path).read_bytes()).hexdigest() for path in paths}
environment = dict(os.environ, B=str(root / 'build/llvm-20261001/source/build'))
result = subprocess.run(['python3', 'test/toolchain.py'], cwd=root, env=environment,
                        text=True, capture_output=True)
(artifacts / 'toolchain.log').write_text(result.stdout + result.stderr)
assert hashes == {path: hashlib.sha256((root / path).read_bytes()).hexdigest() for path in paths}
tree = ast.parse((root / 'test/toolchain.py').read_text())
population = {(node.name, method.name) for node in tree.body if isinstance(node, ast.ClassDef)
              for method in node.body if isinstance(method, ast.FunctionDef)
              and method.name.startswith('test_')}
baseline_tree = ast.parse((root / 'build/llvm-20261001/source/test/toolchain.py').read_text())
baseline = {(node.name, method.name) for node in baseline_tree.body if isinstance(node, ast.ClassDef)
            for method in node.body if isinstance(method, ast.FunctionDef)
            and method.name.startswith('test_')}
assert baseline <= population
assert len(population) == 130
assert 'Ran 130 tests' in result.stderr and result.returncode == 0
assert '\nOK\n' in result.stderr and 'skipped=' not in result.stderr
revision = subprocess.run(['git', '-C', '/home/cosmo/Git/BCApps', 'rev-parse', 'main'],
                          capture_output=True, text=True, check=True).stdout.strip()
receipt = {'controls': controls, 'independent_token_inventory_agrees': True,
           'toolchain_exit': result.returncode, 'toolchain_green': len(population),
           'new_toolchain_methods': sorted(population - baseline), 'source_hashes': hashes,
           'bcapps_main': revision, 'ut_codeunits': len(after), 'ut_methods': 2310,
           'ut_identity_gains': 0, 'ut_identity_losses': 0,
           'configured_build': environment['B'], 'G1_proved': False}
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
