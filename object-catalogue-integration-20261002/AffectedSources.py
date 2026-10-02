from collections import defaultdict, deque
import json
from pathlib import Path
import re

root = Path(__file__).resolve().parent
source = root / 'source'
app_roots = sorted(path for path in (source / 'apps').iterdir() if path.is_dir())
roots = [source / 'include', *app_roots]
changed = {source / 'include/platform' / name for name in ('AllObj.h', 'AllObjWithCaption.h', 'AllObjType.h')}
paths = list((source / 'include').rglob('*.h'))
paths += [path for path in (source / 'apps').rglob('*') if path.suffix in ('.h', '.cpp')]
reverse = defaultdict(set)
direct = []
for path in paths:
    text = path.read_text()
    if path.suffix == '.cpp' and re.search(r'AllObjWithCaption|platform::AllObj\b|platform::AllObjType\b', text):
        direct.append(path.relative_to(source).as_posix())
    for target in re.findall(r'^\s*#\s*include\s*"([^"]+)"', text, re.M):
        for directory in [path.parent, *roots]:
            candidate = directory / target
            if candidate.is_file():
                reverse[candidate.resolve()].add(path.resolve())
                break
walked = set(path.resolve() for path in changed)
queue = deque(walked)
while queue:
    for consumer in reverse[queue.popleft()] - walked:
        walked.add(consumer)
        queue.append(consumer)
cpp = sorted(path.relative_to(source).as_posix() for path in walked if path.suffix == '.cpp')
receipt = {'changed_headers': sorted(path.relative_to(source).as_posix() for path in changed),
           'generated_cpp_population': sum(path.suffix == '.cpp' for path in paths),
           'quoted_include_transitive_cpp': cpp, 'direct_symbol_cpp': sorted(direct),
           'include_graph_is_diagnostic_not_compiler_dependency_proof': True}
(root / 'artifacts/affected-sources.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'cpp_population': receipt['generated_cpp_population'], 'transitive_cpp': len(cpp),
                  'direct_cpp': len(direct)}))
