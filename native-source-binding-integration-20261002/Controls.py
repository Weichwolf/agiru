import importlib.util
import json
import os
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
source = task / 'source'
main = task.parent.parent
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identities = {str(tree): verify.digest(tree) for tree in (main, source)}
program = r'''
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import unittest
spec = importlib.util.spec_from_file_location('toolchain', os.environ['CONTROL_SOURCE'] + '/test/toolchain.py')
test = importlib.util.module_from_spec(spec)
spec.loader.exec_module(test)
root = Path(os.environ['CONTROL_BEFORE'])
after_compiler = str(test.SCRIPT.parents[1] / 'build/agirutc')
before = sys.argv[1] == 'before'
if before:
    original = subprocess.run
    def run(command, *args, **kwargs):
        if command and str(command[0]) == after_compiler:
            command = [str(root / 'build/agirutc'), *command[1:]]
            kwargs['env'] = dict(os.environ, LD_LIBRARY_PATH=str(root / 'build'))
        return original(command, *args, **kwargs)
    subprocess.run = run
    symbols_spec = importlib.util.spec_from_file_location('old_symbols', root / 'scripts/fetch_symbols.py')
    test.symbols = importlib.util.module_from_spec(symbols_spec)
    symbols_spec.loader.exec_module(test.symbols)
suite = unittest.defaultTestLoader.loadTestsFromTestCase(test.SystemSourceBindingGate)
suite.addTest(test.SymbolsPackageGate('test_verification_entrypoint_is_offline_and_read_only'))
result = unittest.TextTestRunner(verbosity=2).run(suite)
print('RECEIPT: ' + json.dumps(dict(tests=result.testsRun, failures=len(result.failures),
    errors=len(result.errors), skipped=len(result.skipped))))
sys.exit(not result.wasSuccessful())
'''
rows = []
for label in ('before', 'after'):
    result = subprocess.run(['python3', '-c', program, label], cwd=source,
        env=dict(os.environ, CONTROL_SOURCE=str(source), CONTROL_BEFORE=str(main), B='build'),
        capture_output=True, text=True)
    text = result.stdout + result.stderr
    (artifacts / ('controls-' + label + '.log')).write_text(text)
    row = json.loads(next(line.removeprefix('RECEIPT: ') for line in text.splitlines()
                         if line.startswith('RECEIPT: ')))
    rows.append(dict(label=label, exit=result.returncode, **row))
    print(json.dumps(rows[-1]), flush=True)
receipt = dict(rows=rows, source_hashes=identities,
    sources_unchanged=all(verify.digest(Path(tree)) == identity for tree, identity in identities.items()),
    before_compiler=str(main / 'build/agirutc'), same_test_bodies=True,
    native_mutant_cases=9, G1_proved=False)
(artifacts / 'controls.json').write_text(json.dumps(receipt, indent=2) + '\n')
assert receipt['sources_unchanged'] and rows[0]['exit'] != 0 and rows[1]['exit'] == 0
assert rows[0]['tests'] == rows[1]['tests'] == 5 and rows[1]['skipped'] == 0
