from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunRealConsumer.py'
text = template.read_text()
before = 'apps/base/system/environment/configuration/page/ReportSettings.cpp'
after = 'apps/base/system/integration/page/ODataEDMDefinitionCard.cpp'
if text.count(before) != 1:
    raise SystemExit('predecessor consumer helper changed')
exec(compile(text.replace(before, after), __file__, 'exec'), globals())
