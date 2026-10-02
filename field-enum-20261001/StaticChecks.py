from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parent
if len(sys.argv) != 2:
    raise SystemExit('expected the prohibited-name search pattern')
rows = []
for scope in (root.parents[1], root / 'source'):
    result = subprocess.run(['grep', '-Ril', '--exclude-dir=.git', '--exclude-dir=build',
                             '--exclude-dir=work', sys.argv[1], str(scope)],
                            text=True, capture_output=True)
    rows.append({'scope': str(scope), 'exit': result.returncode,
                 'matches': result.stdout.splitlines(), 'errors': result.stderr,
                 'valid': result.returncode == 1 and not result.stdout and not result.stderr})
board = root.parents[1] / 'board'
names = ('README.md', '0034_every_object_kind_will_have_a_truthful_translation_and_runtime_census.md',
         '0073_generated_expressions_will_preserve_al_types_and_evaluation_effects.md')
whitespace = {name: [number for number, line in enumerate((board / name).read_text().splitlines(), 1)
                      if line.rstrip() != line] for name in names}
receipt = {'name_scan': rows, 'board_trailing_whitespace': whitespace,
           'success': all(row['valid'] for row in rows) and not any(whitespace.values())}
(root / 'artifacts/static-checks.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'success': receipt['success'], 'matching_files': sum(len(row['matches']) for row in rows)}))
raise SystemExit(0 if receipt['success'] else 1)
