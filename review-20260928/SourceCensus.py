import collections
import importlib.util
import json
from pathlib import Path
import re

root = Path('/home/cosmo/Git/BCApps/src')
spec = importlib.util.spec_from_file_location('manifest', 'scripts/ut_manifest.py')
manifest = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manifest)
apps = collections.defaultdict(lambda: {'codeunits': 0, 'test_attributes': 0, 'recognized_methods': 0})
mismatches = []
encodings = collections.Counter()
unreadable = []
all_count = 0
for path in sorted(root.rglob('*.al')):
    all_count += 1
    data = path.read_bytes()
    encoding = 'utf-16' if data.startswith((b'\xff\xfe', b'\xfe\xff')) else 'utf-8-sig'
    encodings[encoding] += 1
    try:
        raw = data.decode(encoding)
    except UnicodeError as error:
        unreadable.append({'source': str(path.relative_to(root)), 'error': str(error)})
        continue
    text = manifest.IGNORED.sub(lambda m: '\n' * m.group().count('\n') + ' ', raw)
    objects = list(manifest.OBJECT.finditer(text))
    for index, obj in enumerate(objects):
        body = text[obj.end():objects[index + 1].start() if index + 1 < len(objects) else len(text)]
        if not re.search(r'\bSubtype\s*=\s*Test\s*;', body, re.I):
            continue
        attrs = len(re.findall(r'\[Test\]', body, re.I))
        methods = list(manifest.METHOD.finditer(body))
        app = next((parent for parent in path.parents if (parent / 'app.json').is_file()), root)
        key = str(app.relative_to(root))
        apps[key]['codeunits'] += 1
        apps[key]['test_attributes'] += attrs
        apps[key]['recognized_methods'] += len(methods)
        if attrs != len(methods):
            mismatches.append({'source': str(path.relative_to(root)), 'attributes': attrs, 'methods': len(methods)})
print(json.dumps({'scope': 'BCApps/src raw source, all Subtype=Test codeunits; conditional branches retained; not an executable manifest',
                  'al_files': all_count, 'encodings': dict(encodings), 'unreadable': unreadable, 'app_roots': len(apps),
                  'totals': {key: sum(app[key] for app in apps.values()) for key in ('codeunits', 'test_attributes', 'recognized_methods')},
                  'mismatches': mismatches, 'apps': dict(sorted(apps.items()))}, indent=2))
