from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunLint.py'
text = template.read_text()
before = "('CodeunitWriter', 'TableWriter')"
after = "('CodeunitWriter',)"
if text.count(before) != 1:
    raise SystemExit('predecessor lint helper changed')
exec(compile(text.replace(before, after), __file__, 'exec'), globals())
