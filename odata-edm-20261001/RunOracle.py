from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunOracle.py'
text = template.read_text()
pairs = [
    ("('StoredFlag.Table.al', 'SavedOptions.Codeunit.al')", "('OData.Codeunit.al',)"),
    ("'a2b15043-9dac-4b2d-a3f1-acd7e9271d38'", "'8c8e7f7a-3e7a-4cc8-b8a1-ddbaf9eb7cf0'"),
    ("'Object Options Oracle'", "'OData Edm Type Oracle'"),
    ("[{'from': 50231, 'to': 50232}]", "[{'from': 50251, 'to': 50251}]"),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor oracle helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
