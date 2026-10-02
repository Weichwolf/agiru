from pathlib import Path
import json
import importlib.util
import subprocess
import tempfile

root = Path(__file__).resolve().parent
transpiler = Path('/home/cosmo/Git/agiru/build/implicit-key-20261001/source/build/agirutc')
receipts = []
spec = importlib.util.spec_from_file_location('ut_manifest', root / 'source/scripts/ut_manifest.py')
manifest = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manifest)
with tempfile.TemporaryDirectory(prefix='agiru-write-status-') as folder:
    fixture = Path(folder)
    (fixture / 'source').mkdir()
    (fixture / 'apps.json').write_text(json.dumps({
        'apps': [{'name': 'fixture', 'source': 'source', 'depends': []}]}))
    (fixture / 'scope.json').write_text(json.dumps({
        'include': ['Microsoft.Fixture'], 'exclude': []}))
    (fixture / 'source/Output.Codeunit.al').write_text('''namespace Microsoft.Fixture;
codeunit 50192 "Output UT" {
    Subtype = Test;
    [Test] procedure SourceCounted() begin end;
}
''')
    command = [str(transpiler), str(fixture), str(fixture / 'apps.json'),
               str(fixture / 'generated')]
    independent = manifest.scan(fixture / 'source')
    for name in ('valid', 'directory-in-place-of-header'):
        result = subprocess.run(command, text=True, capture_output=True, timeout=30)
        headers = list((fixture / 'generated').rglob('OutputUT.h'))
        if len(headers) != 1:
            raise RuntimeError('fixture header population changed')
        header = headers[0]
        receipts.append({'control': name, 'exit': result.returncode,
                         'source_codeunits': len(independent),
                         'source_test_methods': sum(len(unit['methods']) for unit in independent),
                         'transpiler_ut_population_reported': 'UT         ' in result.stdout,
                         'header_is_file': header.is_file(), 'header_is_directory': header.is_dir(),
                         'sentinel_preserved': (header / 'sentinel').is_file(),
                         'stdout': result.stdout, 'stderr': result.stderr})
        if name == 'valid':
            if result.returncode != 0 or not header.is_file():
                raise RuntimeError('positive control failed')
            header.unlink()
            header.mkdir()
            (header / 'sentinel').write_text('fixture data must survive')
artifacts = root / 'artifacts'
artifacts.mkdir(exist_ok=True)
(artifacts / 'write-status-before.json').write_text(json.dumps(receipts, indent=2) + '\n')
print(json.dumps([{key: row[key] for key in ('control', 'exit', 'header_is_file', 'header_is_directory', 'sentinel_preserved')}
                  for row in receipts], indent=2))
