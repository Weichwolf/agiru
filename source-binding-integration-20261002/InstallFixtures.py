from pathlib import Path

task = Path(__file__).resolve().parent
fixture = task / 'fixtures'
destination = task / 'source/test/source-binding'
patch = ['*** Begin Patch']
for path in sorted(fixture.rglob('*')):
    if not path.is_file():
        continue
    target = destination / path.relative_to(fixture)
    assert not target.exists()
    patch.append('*** Add File: ' + str(target))
    patch.extend('+' + line for line in path.read_text().splitlines())
patch.append('*** End Patch')
print('\n'.join(patch))
