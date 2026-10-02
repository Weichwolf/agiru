from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunRealConsumer.py'
text = template.read_text()
pairs = [
    ('apps/base/system/environment/configuration/page/ReportSettings.cpp',
     'apps/base/system/visualization/page/CopyGenericChart.cpp'),
    ("'Refused' in result.stderr and 'Variable = T{};' in result.stderr",
     "'Refused' in result.stderr and 'Caption' in result.stderr"),
    ("'sql_or_full_saved_settings_workflow_proof': False", "'sql_or_full_chart_copy_workflow_proof': False"),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor consumer helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
