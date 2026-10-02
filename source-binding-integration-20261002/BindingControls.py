import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch

task = Path(__file__).resolve().parent
source = task / 'source'
main = task.parent.parent
artifacts = task / 'artifacts'
if len(sys.argv) > 1:
    artifacts = artifacts / sys.argv[1]
    artifacts.mkdir(exist_ok=True)
os.environ['B'] = str(source / 'build')
spec = importlib.util.spec_from_file_location('toolchain', source / 'test/toolchain.py')
tests = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tests)
real_run = subprocess.run
before_compiler = task / 'before-source/build/agirutc'
if not before_compiler.exists():
    before_compiler = main / 'build/agirutc'

def prior_transpiler(command, *args, **kwargs):
    if str(command[0]) == str(source / 'build/agirutc'):
        command = [str(before_compiler), *command[1:]]
        kwargs['env'] = dict(kwargs.get('env', os.environ),
            LD_LIBRARY_PATH=str(before_compiler.parent))
    return real_run(command, *args, **kwargs)

rows = {}
for label, replacement in [('before', prior_transpiler), ('after', real_run)]:
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(tests.TableSourceBindingGate)
    with (artifacts / ('binding-controls-' + label + '.log')).open('w') as log:
        with patch.object(tests.subprocess, 'run', replacement):
            result = unittest.TextTestRunner(stream=log, verbosity=2).run(suite)
    rows[label] = dict(tests=result.testsRun, failures=len(result.failures), errors=len(result.errors),
        skipped=len(result.skipped), failed_identities=[test.id() for test, _ in result.failures],
        error_identities=[test.id() for test, _ in result.errors], passed=result.wasSuccessful())
receipt = dict(results=rows, identical_test_source=True, used_pch=False,
    before_compiler=str(before_compiler), after_compiler=str(source / 'build/agirutc'))
(artifacts / 'binding-controls.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert not rows['before']['passed'] and rows['after']['passed']
assert rows['before']['tests'] == rows['after']['tests'] == 2
assert rows['before']['skipped'] == rows['after']['skipped'] == 0
