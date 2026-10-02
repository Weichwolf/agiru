from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunHeaderCost.py'
text = template.read_text()
pairs = [
    ("str(root / 'HeaderCost.cpp')", "str(image / 'src/rt/PlatformTables.cpp')"),
    ("'single_header_only': True", "'native_registration_unit_only': True"),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor header-cost helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
