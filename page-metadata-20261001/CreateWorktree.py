from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/CreateWorktree.py'
text = template.read_text()
before = "origin = Path('/home/cosmo/Git/agiru/build/field-enum-20261001/source')"
after = "origin = Path('/home/cosmo/Git/agiru/build/object-options-20261001/source')"
if text.count(before) != 1:
    raise SystemExit('predecessor copy helper changed')
exec(compile(text.replace(before, after), __file__, 'exec'), globals())
