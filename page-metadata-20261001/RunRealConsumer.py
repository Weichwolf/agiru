from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunRealConsumer.py'
text = template.read_text()
pairs = [
    ("apps/base/system/environment/configuration/page/ReportSettings.cpp", "apps/base/utilities/codeunit/PageManagement.cpp"),
    ("'Refused' in result.stderr and 'Variable = T{};' in result.stderr", "'native field count mismatch: Page Metadata' in result.stderr"),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor consumer helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
