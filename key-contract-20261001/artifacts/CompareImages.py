from pathlib import Path
import hashlib
import json

root = Path(__file__).resolve().parent
before = root / 'apps-before'
after = root.parents[1] / 'apps'
def image(folder):
    return {str(path.relative_to(folder)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in folder.rglob('*') if path.is_file()}
old, new = image(before), image(after)
changed = sorted(name for name in old.keys() & new.keys() if old[name] != new[name])
print(json.dumps({'before': len(old), 'after': len(new),
                  'missing': sorted(old.keys() - new.keys()), 'added': sorted(new.keys() - old.keys()),
                  'changed': changed,
                  'changed_headers': sum(name.endswith('.h') for name in changed),
                  'changed_definitions': sum(name.endswith('.def.cpp') for name in changed),
                  'changed_bodies': sum(name.endswith('.cpp') and not name.endswith('.def.cpp')
                                        for name in changed)}, indent=2))
