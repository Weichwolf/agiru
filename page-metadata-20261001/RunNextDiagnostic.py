from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunNextDiagnostic.py'
text = template.read_text()
pairs = [
    ('apps/base/utilities/codeunit/PageManagement.cpp',
     'apps/base/system/integration/page/ODataEDMDefinitionCard.cpp'),
    ("result.returncode != 0 and 'native field count mismatch: Page Metadata' in result.stderr and\n                 'native field declaration mismatch: Page Metadata.Caption' in result.stderr",
     "result.returncode != 0 and 'Refused' in result.stderr and 'Variable = T{};' in result.stderr"),
    ("'source_declared_fields': 32", "'field': 'EdmXml', 'missing_native_table_binding': True"),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor diagnostic helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
