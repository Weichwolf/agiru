from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunBuilds.py'
exec(compile(template.read_text(), __file__, 'exec'), globals())
