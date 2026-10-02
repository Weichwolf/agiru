import ast
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import subprocess
import unittest
from unittest.mock import patch

root = Path(__file__).resolve().parents[2]
artifacts = Path(__file__).resolve().parent
image = root / 'build/page-source-binding-20261001/source'


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    loaded = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(loaded)
    return loaded


tests = module('toolchain', root / 'test/toolchain.py')
previous = module('previous_milestone', image / 'scripts/ut_milestone.py')
current = tests.milestone
controls = []
for label, implementation in (('previous', previous), ('current', current)):
    output = io.StringIO()
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(tests.SourceRevisionGate)
    with patch.object(tests, 'milestone', implementation):
        result = unittest.TextTestRunner(stream=output, verbosity=2).run(suite)
    (artifacts / (label + '-controls.log')).write_text(output.getvalue())
    controls.append({'implementation': label, 'tests': result.testsRun,
                     'failures': len(result.failures), 'errors': len(result.errors),
                     'skipped': len(result.skipped)})
assert [(row['tests'], row['failures'], row['errors']) for row in controls] == [(5, 2, 0), (5, 0, 0)]

environment = dict(os.environ)
environment.pop('AGIRU_BC_REVISION', None)
frozen = root / 'build/compiler-llvm-20261001/bc_source'
checkout = Path('/home/cosmo/Git/BCApps/src')
with patch.dict(os.environ, environment, clear=True), patch.object(previous, 'ROOT', image), \
        patch.object(current, 'ROOT', image):
    real = {'previous_frozen': previous.bc_source_revision(frozen),
            'current_frozen': current.bc_source_revision(frozen),
            'previous_checkout': previous.bc_source_revision(checkout),
            'current_checkout': current.bc_source_revision(checkout)}
assert real['previous_frozen'] == current.source_revision(root)
assert real['current_frozen'] is None
assert real['previous_checkout'] == real['current_checkout'] == 'a9ea4d84534cebba852c44bf0f841c2ea149de4e'
manifest = module('manifest', root / 'scripts/ut_manifest.py')
before = module('previous_manifest', image / 'scripts/ut_manifest.py').scan(frozen / 'Layers/W1/Tests')
after = manifest.scan(frozen / 'Layers/W1/Tests')
assert before == after and len(after) == 80 and sum(len(row['methods']) for row in after) == 2310
paths = ('scripts/ut_milestone.py', 'test/toolchain.py')
hashes = {path: hashlib.sha256((root / path).read_bytes()).hexdigest() for path in paths}
environment['B'] = str(root / 'build/llvm-20261001/source/build')
result = subprocess.run(['python3', 'test/toolchain.py'], cwd=root, env=environment,
                        text=True, capture_output=True)
(artifacts / 'toolchain.log').write_text(result.stdout + result.stderr)
assert result.returncode == 0 and 'Ran 135 tests' in result.stderr and '\nOK\n' in result.stderr
assert 'skipped=' not in result.stderr
assert hashes == {path: hashlib.sha256((root / path).read_bytes()).hexdigest() for path in paths}


def methods(path):
    tree = ast.parse(path.read_text())
    return {(row.name, method.name) for row in tree.body if isinstance(row, ast.ClassDef)
            for method in row.body if isinstance(method, ast.FunctionDef)
            and method.name.startswith('test_')}


baseline = methods(root / 'build/llvm-20261001/source/test/toolchain.py')
prior = json.loads((root / 'build/manifest-boundary-20261001/proof.json').read_text())
baseline.update(tuple(row) for row in prior['new_toolchain_methods'])
population = methods(root / 'test/toolchain.py')
assert len(baseline) == 130 and baseline <= population and len(population) == 135
receipt = {'source_hashes': hashes, 'toolchain_green': 135, 'toolchain_skipped': 0,
           'all_previous_toolchain_identities_retained': True, 'controls': controls,
           'new_toolchain_methods': sorted(population - baseline), 'actual_source_revisions': real,
           'unknown_frozen_revision_requires_explicit_producer_provenance': True,
           'ut_codeunits': 80, 'ut_methods': 2310, 'ut_identity_gains': 0, 'ut_identity_losses': 0,
           'historical_receipts_rewritten': False, 'G1_proved': False}
(artifacts / 'proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
