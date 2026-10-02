from pathlib import Path
import importlib.util
import json

root = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('ut_manifest', root / 'scripts/ut_manifest.py')
manifest = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manifest)
snapshot = Path('/home/cosmo/Git/agiru/build/verify/20260930T210740Z-797012')
before = json.loads((snapshot / 'artifacts/ut.log.manifest.json').read_text())
after = manifest.scan(Path('/home/cosmo/Git/BCApps/src/Layers/W1/Tests'))


def population(entries):
    return {(entry['id'], entry['name'], method)
            for entry in entries for method in entry['methods']}


old, new = population(before), population(after)
print(json.dumps({'snapshot': snapshot.name,
                  'before_codeunits': len(before), 'after_codeunits': len(after),
                  'before_methods': len(old), 'after_methods': len(new),
                  'missing': sorted(old - new), 'added': sorted(new - old),
                  'execution_proof': False}, indent=2))
