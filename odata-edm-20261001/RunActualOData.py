from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'page-metadata-20261001/RunActualPageManagement.py'
text = template.read_text()
pairs = [
    ('actual-page-management-', 'actual-odata-'),
    ("'ActualPageManagement.cpp'", "'ActualOData.cpp'"),
    ('apps/base/utilities/codeunit/PageManagement.cpp', 'apps/base/system/integration/page/ODataEDMDefinitionCard.cpp'),
    ("'2 check(s), 0 red'", "'12 check(s), 0 red'"),
    ("'zero_table_shortcuts_only': True", "'temporary_store_only': True, 'fixture_explicitly_installs_temporary_state': True"),
    ('artifacts/actual-page-management.json', 'artifacts/actual-odata.json'),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor actual-consumer helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
