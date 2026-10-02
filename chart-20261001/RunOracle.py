from pathlib import Path
import json
import re

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunOracle.py'
text = template.read_text()
pairs = [
    ("('StoredFlag.Table.al', 'SavedOptions.Codeunit.al')", "('Chart.Codeunit.al',)"),
    ("'a2b15043-9dac-4b2d-a3f1-acd7e9271d38'", "'0b18b00c-4259-49e0-a9bd-cd4f2c159d42'"),
    ("'Object Options Oracle'", "'Chart Oracle'"),
    ("[{'from': 50231, 'to': 50232}]", "[{'from': 50261, 'to': 50261}]"),
    (", '/warnaserror+'", ""),
    ("print(json.dumps({'exit': result.returncode, 'stdout': result.stdout, 'stderr': result.stderr}), flush=True)",
     "receipt['warnings'] = re.findall(r'warning (AL[0-9]+): ([^\\n]+)', result.stdout + result.stderr)\n"
     "receipt['warning_policy'] = 'retain all original Pending warnings; no suppression'\n"
     "(root / 'artifacts/al-oracle.json').write_text(json.dumps(receipt, indent=2) + '\\n')\n"
     "print(json.dumps({'exit': result.returncode, 'stdout': result.stdout, 'stderr': result.stderr, 'warnings': receipt['warnings']}), flush=True)"),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor oracle helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
