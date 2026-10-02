from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/RunCatalogue.py'
text = template.read_text()
before = "{'PageMetadata', 'TenantLicenseState'}"
after = "{'TenantLicenseState'}"
if text.count(before) != 1:
    raise SystemExit('predecessor catalogue helper changed')
exec(compile(text.replace(before, after), __file__, 'exec'), globals())
