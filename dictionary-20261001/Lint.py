import json
import os
from pathlib import Path
import re
import subprocess
import sys

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
units = ['src/net/Generic.cpp', 'src/gen/Door.cpp', 'src/gen/BodyWriter.cpp',
         'test/gate/GenTableBindingGate.cpp', 'test/gate/DictionaryGate.cpp',
         'test/gate/DesignerConstantsGate.cpp']
parents = {'before': root.parent / 'page-source-binding-20261001/source', 'after': root / 'source'}
rows = []
label_prefix = sys.argv[1] if len(sys.argv) > 1 else 'initial'
inherited_headers = set()
for unit in units:
    comparisons = {}
    for label, source in parents.items():
        if label == 'before' and not (source / unit).exists():
            continue
        command = ['make', 'lint-one', 'UNIT=' + unit, 'JOBS=2']
        result = subprocess.run(command, cwd=source, capture_output=True, text=True,
                                env=dict(os.environ, CCACHE_DIR=str(root / 'ccache')))
        stem = 'lint-' + label_prefix + '-' + label + '-' + Path(unit).stem
        (artifacts / (stem + '.make.log')).write_text(result.stdout + result.stderr)
        log = source / 'build/lint/targeted.log'
        assert log.exists(), result.stdout + result.stderr
        text = log.read_text()
        (artifacts / (stem + '.log')).write_text(text)
        diagnostics = {re.sub(r':\d+:\d+:', ':LOC:', line.replace(str(source), '<source>'))
                       for line in text.splitlines() if ': error:' in line}
        comparisons[label] = {'exit': result.returncode, 'findings': sorted(diagnostics)}
        if label == 'before':
            inherited_headers.update(line for line in diagnostics if line.startswith('<source>/include/'))
    before = set(comparisons.get('before', {}).get('findings', []))
    after = set(comparisons['after']['findings'])
    inherited = after.intersection(inherited_headers) if unit.startswith('test/') else set()
    row = {'unit': unit, 'results': comparisons, 'inherited_header_findings': sorted(inherited),
           'new_findings': sorted(after - before - inherited), 'removed_findings': sorted(before - after)}
    rows.append(row)
    print(json.dumps({'unit': unit, 'before': len(before), 'after': len(after),
                      'new': row['new_findings']}), flush=True)
(artifacts / ('lint-' + label_prefix + '-comparison.json')).write_text(json.dumps(rows, indent=2) + '\n')
assert not any(row['new_findings'] for row in rows), rows
