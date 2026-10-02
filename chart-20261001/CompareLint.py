from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'field-enum-20261001/CompareLint.py'
text = template.read_text()
before = "('src/gen/BodyWriter.cpp', 'src/gen/CodeunitWriter.cpp')"
after = "('src/gen/CodeunitWriter.cpp', 'src/gen/PageWriter.cpp', 'src/gen/Door.cpp', 'src/rt/PlatformTables.cpp')"
if text.count(before) != 1:
    raise SystemExit('predecessor lint helper changed')
exec(compile(text.replace(before, after), __file__, 'exec'), globals())
