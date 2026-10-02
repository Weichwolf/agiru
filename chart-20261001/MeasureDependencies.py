from pathlib import Path
import json
import re

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
results = {}
for label, image in [('predecessor', origin), ('current', source)]:
    files = directives = redundant = 0
    duplicated = []
    for path in sorted((image / 'apps').rglob('*')):
        if path.suffix not in ('.h', '.cpp'):
            continue
        files += 1
        includes = re.findall(r'^#include ([^\n]+)$', path.read_text(), re.M)
        directives += len(includes)
        repeated = len(includes) - len(set(includes))
        redundant += repeated
        if repeated:
            duplicated.append({'path': str(path.relative_to(image)), 'redundant': repeated})
    results[label] = {'files': files, 'directives': directives,
                      'redundant_directives': redundant, 'files_with_duplicates': len(duplicated),
                      'remaining_duplicates': duplicated}
receipt = {'results': results, 'same_file_population': results['predecessor']['files'] == results['current']['files'],
           'redundancies_decreased': results['current']['redundant_directives'] < results['predecessor']['redundant_directives'],
           'all_dependencies_unique': results['current']['redundant_directives'] == 0,
           'throughput_or_erp_performance_claim': False}
(root / 'artifacts/dependencies.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({label: {key: value for key, value in result.items() if key != 'remaining_duplicates'}
                  for label, result in results.items()}), flush=True)
if not receipt['same_file_population'] or not receipt['redundancies_decreased']:
    raise SystemExit('generated dependency population or duplicate baseline regressed')
