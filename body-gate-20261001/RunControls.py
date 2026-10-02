from pathlib import Path
import importlib.util
import json
import shutil
import sys
import unittest
from unittest.mock import patch

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path('/home/cosmo/Git/agiru/build/native-catalogue-20261001/source')
spec = importlib.util.spec_from_file_location('toolchain', source / 'test/toolchain.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
copy = shutil.copy2
mode = sys.argv[1]
if mode not in ('old', 'current'):
    raise SystemExit('expected old or current')

def selected_copy(path, destination, *args, **kwargs):
    path = Path(path)
    if mode == 'old' and path in (source / 'Makefile', source / 'scripts/first_gap.sh'):
        path = origin / path.relative_to(source)
    return copy(path, destination, *args, **kwargs)

suite = unittest.defaultTestLoader.loadTestsFromTestCase(module.FirstGapGate)
with patch.object(module.shutil, 'copy2', selected_copy):
    result = unittest.TextTestRunner(verbosity=2).run(suite)
receipt = {'mode': mode, 'tests': result.testsRun,
           'failures': [str(case) for case, _ in result.failures],
           'errors': [str(case) for case, _ in result.errors],
           'skipped': [str(case) for case, _ in result.skipped],
           'success': result.wasSuccessful(), 'real_make_and_clang': True}
artifacts = root / 'artifacts'
artifacts.mkdir(exist_ok=True)
(artifacts / f'controls-{mode}.json').write_text(json.dumps(receipt, indent=2) + '\n')
raise SystemExit(0 if result.wasSuccessful() else 1)
