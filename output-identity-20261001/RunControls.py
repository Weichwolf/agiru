from pathlib import Path
import importlib.util
import json
import os
import subprocess
import sys
import unittest
from unittest.mock import patch

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path('/home/cosmo/Git/agiru/build/write-status-20261001/source')
mode = sys.argv[1]
if mode not in ('current', 'old-generator'):
    raise SystemExit('unknown control')
spec = importlib.util.spec_from_file_location('toolchain', source / 'test/toolchain.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
run = subprocess.run
commands = []
build = (source / os.environ.get('B', 'build')).resolve()
old_build = origin / ('build/gcc' if build.name == 'gcc' else 'build')
artifacts = root / 'artifacts'
artifacts.mkdir(exist_ok=True)

def selected_run(arguments, *args, **kwargs):
    arguments = list(arguments)
    if mode == 'old-generator' and arguments[0] == str(build / 'agirutc'):
        arguments[0] = str(old_build / 'agirutc')
    result = run(arguments, *args, **kwargs)
    commands.append({'arguments': arguments, 'exit': result.returncode,
                     'stdout': result.stdout, 'stderr': result.stderr})
    return result

suite = unittest.defaultTestLoader.loadTestsFromTestCase(module.TranspilerOwnershipGate)
with patch.object(module.subprocess, 'run', selected_run):
    result = unittest.TextTestRunner(verbosity=2).run(suite)
receipt = {'mode': mode, 'build': str(build), 'tests': result.testsRun,
           'failures': [str(case) for case, _ in result.failures],
           'errors': [str(case) for case, _ in result.errors],
           'skipped': [str(case) for case, _ in result.skipped],
           'success': result.wasSuccessful(), 'commands': commands}
compiler = 'gcc' if build.name == 'gcc' else 'clang'
(artifacts / f'generated-{mode}-{compiler}.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'compiler': compiler, 'mode': mode, 'tests': result.testsRun,
                  'failures': len(result.failures), 'errors': len(result.errors)}))
raise SystemExit(0 if result.wasSuccessful() else 1)
