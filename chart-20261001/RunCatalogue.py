from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunCatalogue.py'
text = template.read_text()
pairs = [
    ('tables 223, binding candidates 17', 'tables 223, binding candidates 19'),
    ('len(rows) == 17', 'len(rows) == 19'),
    ("{'PageMetadata', 'TenantLicenseState'}", "{'TenantLicenseState'}"),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor contract helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
