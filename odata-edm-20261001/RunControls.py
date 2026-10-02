from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunControls.py'
text = template.read_text()
pairs = [('125)', '141)'), ('1022)]', '1328)]')]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor control helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
