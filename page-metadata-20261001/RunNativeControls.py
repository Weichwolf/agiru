from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunControls.py'
text = template.read_text()
before = "[('GenTableBinding', ['gen', 'al'], 125),\n                               ('NativeObject', ['rt', 'net', 'db'], 1022)]"
after = "[('NativeObject', ['rt', 'net', 'db'], 1294)]"
if text.count(before) != 1:
    raise SystemExit('predecessor native control helper changed')
exec(compile(text.replace(before, after), __file__, 'exec'), globals())
