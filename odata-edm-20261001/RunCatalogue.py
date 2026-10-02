from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunCatalogue.py'
text = template.read_text()
pairs = [
    ("binding candidates 17'", "binding candidates 18'"),
    ('len(rows) == 17', 'len(rows) == 18'),
    ("{'PageMetadata', 'TenantLicenseState'}", "{'TenantLicenseState'}"),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor catalogue helper changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
