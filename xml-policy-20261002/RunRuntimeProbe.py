import hashlib
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
main = root.parent.parent
result = subprocess.run([str(root / 'runtime-probe'), str(root)], capture_output=True, text=True)
(root / 'runtime-probe.log').write_text(result.stdout + result.stderr)
assert result.returncode == 0, result.stderr
rows = [json.loads(line) for line in result.stdout.splitlines()]
assert len(rows) == 32
assert all(not row['refused'] for row in rows)
leaks = [row for row in rows if row['mode'] in (0, 1) and row['owned_external_marker']]
closed = [row for row in rows if row['closed'] and not row['refused']]
ignored = [row for row in rows if row['mode'] == 1 and row['doctype_reported']]
assert len(leaks) == 4 and len(closed) == 6 and len(ignored) == 8
positioned = [row for row in rows if row['case'] == 'positioned-last']
assert len(positioned) == 2 and all(row['reached_last'] and row['reloads_already_consumed_first'] for row in positioned)
receipt = {'exit': result.returncode, 'test_cases': len(rows), 'input_kind': 'Blob-backed InStream, UTF-8 and UTF-16LE',
           'production_fixed': False, 'prohibit_or_ignore_entity_leaks': len(leaks),
           'closed_reader_document_loads_accepted': len(closed), 'ignored_doctypes_reported': len(ignored),
           'positioned_reader_reloads_consumed_sibling': len(positioned),
           'closed_reader_should_be_empty_not_assumed_to_throw': True,
           'Load_reader_contract': 'https://learn.microsoft.com/en-us/dotnet/api/system.xml.xmldocument.load?view=net-9.0',
           'source_contract': 'dotnet/dotnet b0f34d51fccc69fd334253924abd8d6853fad7aa XmlLoader.cs::Load/LoadDocSequence',
           'source_sha256': hashlib.sha256((root / 'RuntimeProbe.cpp').read_bytes()).hexdigest(),
           'binary_sha256': hashlib.sha256((root / 'runtime-probe').read_bytes()).hexdigest(),
           'production_sources': {name: hashlib.sha256((main / name).read_bytes()).hexdigest() for name in
                                   ('include/dotnet/XmlReader.h', 'src/net/XmlReader.cpp', 'src/net/DotNetXml.cpp', 'src/net/XmlEngine.cpp')},
           'rows': rows}
(root / 'runtime-probe.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items() if key not in ('rows', 'production_sources')}))
