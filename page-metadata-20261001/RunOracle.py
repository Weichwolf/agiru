from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunOracle.py'
text = template.read_text()
pairs = [
    ("('StoredFlag.Table.al', 'SavedOptions.Codeunit.al')", "('Metadata.Codeunit.al',)"),
    ("'a2b15043-9dac-4b2d-a3f1-acd7e9271d38'", "'9e2fa6bb-8238-4cc3-bfc0-4eb9ae4ba383'"),
    ("'Object Options Oracle'", "'Page Metadata Oracle'"),
    ("[{'from': 50231, 'to': 50232}]", "[{'from': 50241, 'to': 50241}]"),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor oracle helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
