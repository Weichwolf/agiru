from pathlib import Path
import difflib
import hashlib
import importlib.util
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
identity = json.loads((root / 'source-identity.json').read_text())
origin = Path(identity['origin'])

def module(name, relative):
    spec = importlib.util.spec_from_file_location(name, source / relative)
    loaded = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(loaded)
    return loaded

verify = module('verify_snapshot', 'scripts/verify_snapshot.py')
manifest = module('ut_manifest', 'scripts/ut_manifest.py')
if verify.digest(origin) != identity['base_source_sha256']:
    raise RuntimeError('predecessor source image changed')
identity.update(source_sha256=verify.digest(source), base_digest_reverified_unchanged=True,
                production_integrated=False,
                prototype_paths=['include/type/StringValue.h', 'src/gen/BodyWriter.cpp',
                                 'test/gate/TextGate.cpp', 'test/toolchain.py::TextConcatenationGate'])
(root / 'source-identity.json').write_text(json.dumps(identity, indent=2) + '\n')
snapshot = Path('/home/cosmo/Git/agiru/build/verify/20260930T210740Z-797012')
before = json.loads((snapshot / 'artifacts/ut.log.manifest.json').read_text())
after = manifest.scan(Path('/home/cosmo/Git/BCApps/src/Layers/W1/Tests'))

def population(entries):
    return {(entry['id'], entry['name'], method)
            for entry in entries for method in entry['methods']}

previous, current = population(before), population(after)
ut = {'snapshot': snapshot.name, 'before_codeunits': len(before), 'after_codeunits': len(after),
      'before_methods': len(previous), 'after_methods': len(current),
      'missing': sorted(previous - current), 'added': sorted(current - previous),
      'execution_proof': False}
(artifacts / 'ut-population.json').write_text(json.dumps(ut, indent=2) + '\n')

def image(folder):
    return {str(path.relative_to(folder)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in folder.rglob('*') if path.is_file()}

old, new = image(origin / 'apps'), image(source / 'apps')
generated = {'before_files': len(old), 'after_files': len(new),
             'missing': sorted(old.keys() - new.keys()), 'added': sorted(new.keys() - old.keys()),
             'changed': sorted(name for name in old.keys() & new.keys() if old[name] != new[name]),
             'bcapps_main': subprocess.check_output(['git', '-C', '/home/cosmo/Git/BCApps',
                                                     'rev-parse', 'main'], text=True).strip(),
             'system_symbols': '/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH'}
(artifacts / 'generated-image.json').write_text(json.dumps(generated, indent=2) + '\n')
patch = []
for name in ['include/type/StringValue.h', 'src/gen/BodyWriter.cpp', 'test/gate/TextGate.cpp',
             'test/toolchain.py']:
    patch.extend(difflib.unified_diff((origin / name).read_text().splitlines(keepends=True),
                                    (source / name).read_text().splitlines(keepends=True),
                                    fromfile='before/' + name, tofile='after/' + name))
(artifacts / 'prototype.patch').write_text(''.join(patch))
print(json.dumps({'source_sha256': identity['source_sha256'], 'ut': ut,
                  'generated_before': len(old), 'generated_after': len(new),
                  'generated_changed': len(generated['changed']),
                  'generated_missing': len(generated['missing']),
                  'generated_added': len(generated['added'])}, indent=2))
