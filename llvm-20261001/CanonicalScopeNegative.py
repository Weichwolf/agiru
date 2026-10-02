import importlib.util
from pathlib import Path
import sys
import unittest

root = Path(__file__).resolve().parent
main = root.parent.parent
sys.path.insert(0, str(main / 'test'))
import toolchain


def archived(name):
    spec = importlib.util.spec_from_file_location(name, root / 'source/scripts' / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


old_manifest = archived('ut_manifest')
toolchain.scope_inventory = archived('scope_inventory')
setup = toolchain.ManifestGate.setUp


def old_setup(self):
    setup(self)
    self.manifest = old_manifest


toolchain.ManifestGate.setUp = old_setup
suite = unittest.TestSuite([
    toolchain.ManifestGate('test_namespace_and_multiple_objects_on_one_line_preserve_ut_identities'),
    toolchain.SourceInventoryGate('test_explicit_product_rules_are_bounded_and_never_replace_raw_counts'),
])
with (root / 'artifacts/canonical-scope-counting-old-controls.log').open('w') as stream:
    result = unittest.TextTestRunner(stream=stream, verbosity=2).run(suite)
assert result.testsRun == 2 and len(result.failures) + len(result.errors) == 2
print('Two counting/path controls fail on the previous implementations.')
