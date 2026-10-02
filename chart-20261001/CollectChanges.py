from pathlib import Path
import difflib
import importlib.util
import json

root = Path(__file__).resolve().parent
source = root / 'source'
identity = json.loads((root / 'source-identity.json').read_text())
origin = Path(identity['origin'])
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
if verify.digest(origin) != identity['base_source_sha256']:
    raise RuntimeError('predecessor source image changed')
previous, current = set(verify.files(origin)), set(verify.files(source))
added, missing = sorted(current - previous), sorted(previous - current)
if added != [Path('include/platform/Chart.h')] or missing:
    raise RuntimeError('unexpected source-path change')
changed = sorted(str(path) for path in current if path not in previous or
                 (origin / path).read_bytes() != (source / path).read_bytes())
handwritten = [path for path in changed if not path.startswith('apps/')]
expected = {'include/platform/Chart.h', 'src/gen/CodeunitWriter.cpp', 'src/gen/CodeunitWriter.h',
            'src/gen/PageWriter.cpp', 'src/gen/Door.cpp', 'src/rt/PlatformTables.cpp',
            'test/gate/NativeObjectGate.cpp', 'test/gate/GenTableBindingGate.cpp',
            'test/gate/GenCodeunitGate.cpp'}
if set(handwritten) != expected:
    raise RuntimeError('unexpected handwritten blast radius: ' + str(handwritten))
generated = [path for path in changed if path.startswith('apps/')]
if not generated or 'apps/absent/absent/Types.h' not in generated:
    raise RuntimeError('expected source-bound regenerated Chart consumers')
if (origin / 'test/slice').read_bytes() != (source / 'test/slice').read_bytes():
    raise RuntimeError('compiling slice changed')
patch = []
for path in changed:
    before = (origin / path).read_text().splitlines(keepends=True) if path in {str(p) for p in previous} else []
    patch.extend(difflib.unified_diff(before, (source / path).read_text().splitlines(keepends=True),
                                    fromfile='before/' + path, tofile='after/' + path))
(root / 'artifacts/prototype.patch').write_text(''.join(patch))
receipt = {'source_sha256': verify.digest(source), 'predecessor_sha256': identity['base_source_sha256'],
           'handwritten_changed': handwritten, 'generated_changed': generated,
           'source_paths_added': [str(path) for path in added], 'source_paths_removed': [],
           'generated_paths_unchanged': all(not str(path).startswith('apps/') for path in added),
           'slice_unchanged': True}
(root / 'artifacts/changes.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
