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
if previous != current:
    raise RuntimeError('source paths added or removed')
changed = sorted(str(path) for path in current if (origin / path).read_bytes() != (source / path).read_bytes())
handwritten = [path for path in changed if not path.startswith('apps/')]
expected = {'include/platform/ObjectOptions.h', 'src/gen/CodeunitWriter.cpp',
            'src/gen/TableWriter.cpp', 'test/gate/NativeObjectGate.cpp',
            'test/gate/GenTableBindingGate.cpp', 'test/toolchain.py'}
if set(handwritten) != expected:
    raise RuntimeError('unexpected handwritten blast radius: ' + str(handwritten))
generated = [path for path in changed if path.startswith('apps/')]
if len(generated) != 26 or 'apps/absent/absent/Types.h' not in generated:
    raise RuntimeError('unexpected generated blast radius: ' + str(generated))
if (origin / 'test/slice').read_bytes() != (source / 'test/slice').read_bytes():
    raise RuntimeError('compiling slice changed')
patch = []
for path in changed:
    patch.extend(difflib.unified_diff((origin / path).read_text().splitlines(keepends=True),
                                    (source / path).read_text().splitlines(keepends=True),
                                    fromfile='before/' + path, tofile='after/' + path))
(root / 'artifacts/prototype.patch').write_text(''.join(patch))
receipt = {'source_sha256': verify.digest(source), 'predecessor_sha256': identity['base_source_sha256'],
           'handwritten_changed': handwritten, 'generated_changed': generated,
           'source_paths_unchanged': True, 'slice_unchanged': True}
(root / 'artifacts/changes.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
