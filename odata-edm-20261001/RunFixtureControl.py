from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunFixtureControl.py'
text = template.read_text()
pairs = [
    ("('StoredFlag.Table.al', 'SavedOptions.Codeunit.al')", "('OData.Codeunit.al',)"),
    ("'Refused' in result.stderr and 'Temporary' in result.stderr",
     "'Refused' in result.stderr and 'Variable = T{};' in result.stderr"),
    ('len(units) == 3', 'len(units) == 1'),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor fixture control helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
