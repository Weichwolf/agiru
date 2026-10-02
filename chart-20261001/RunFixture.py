from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunFixture.py'
text = template.read_text()
pairs = [
    ("('StoredFlag.Table.al', 'SavedOptions.Codeunit.al')", "('Chart.Codeunit.al',)"),
    ("'GeneratedOptions.cpp'", "'GeneratedChart.cpp'"),
    ('len(units) == 3', 'len(units) == 1'),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor fixture helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
