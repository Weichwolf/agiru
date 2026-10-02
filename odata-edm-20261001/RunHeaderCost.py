from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'page-metadata-20261001/RunHeaderCost.py'
exec(compile(template.read_text(), __file__, 'exec'), globals())
