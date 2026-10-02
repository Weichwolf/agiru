from pathlib import Path
import difflib
import hashlib
import importlib.util
import json
import re
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
identity = json.loads((root / 'source-identity.json').read_text())
origin = Path(identity['origin'])

def module(name, relative):
    spec = importlib.util.spec_from_file_location(name, source / relative)
    loaded = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(loaded)
    return loaded

verify = module('verify_snapshot', 'scripts/verify_snapshot.py')
manifest = module('ut_manifest', 'scripts/ut_manifest.py')
if verify.digest(origin) != identity['base_source_sha256']:
    raise RuntimeError('predecessor source image changed')
paths = ['src/tc/Main.cpp', 'test/toolchain.py']
identity.update(source_sha256=verify.digest(source), base_digest_reverified_unchanged=True,
                production_integrated=False, prototype_paths=paths)
(root / 'source-identity.json').write_text(json.dumps(identity, indent=2) + '\n')

def save(name, value):
    (artifacts / f'{name}.json').write_text(json.dumps(value, indent=2) + '\n')

def image(folder):
    return {str(path.relative_to(folder)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in folder.rglob('*') if path.is_file()}

old, new = image(origin / 'apps'), image(source / 'apps')
generated = {'before_files': len(old), 'after_files': len(new),
             'missing': sorted(old.keys() - new.keys()), 'added': sorted(new.keys() - old.keys()),
             'changed': sorted(name for name in old.keys() & new.keys() if old[name] != new[name]),
             'bcapps_main': subprocess.check_output(['git', '-C', '/home/cosmo/Git/BCApps',
                                                     'rev-parse', 'main'], text=True).strip()}
save('generated-image', generated)
before = json.loads((Path('/home/cosmo/Git/agiru/build/verify/20260930T210740Z-797012') /
                     'artifacts/ut.log.manifest.json').read_text())
after = manifest.scan(Path('/home/cosmo/Git/BCApps/src/Layers/W1/Tests'))

def population(entries):
    return {(entry['id'], entry['name'], method)
            for entry in entries for method in entry['methods']}

previous, current = population(before), population(after)
ut = {'before_codeunits': len(before), 'after_codeunits': len(after),
      'before_methods': len(previous), 'after_methods': len(current),
      'missing': sorted(previous - current), 'added': sorted(current - previous),
      'execution_proof': False}
save('ut-population', ut)
patch = []
for name in paths:
    patch.extend(difflib.unified_diff((origin / name).read_text().splitlines(keepends=True),
                                    (source / name).read_text().splitlines(keepends=True),
                                    fromfile='before/' + name, tofile='after/' + name))
(artifacts / 'prototype.patch').write_text(''.join(patch))

def failures(log):
    units = set()
    for line in log.read_text().splitlines():
        match = re.search(r'FAIL\s+.*?/test/gate/([^/:]+)\.cpp:', line)
        escaped = re.search(r'FAIL\s+an (?:unknown )?exception left (\S+)', line)
        if match:
            units.add(match[1].removesuffix('Gate'))
        elif escaped:
            units.add(escaped[1])
    return units

comparison = []
for tag, previous_log in (('clang', 'test-clang-verified.log'), ('gcc', 'test-gcc.log')):
    old_failures = failures(origin.parent / 'artifacts' / previous_log)
    new_failures = failures(artifacts / f'test-{tag}.log')
    comparison.append({'compiler': tag, 'before_failed_cases': len(old_failures),
                       'after_failed_cases': len(new_failures),
                       'added_failures': sorted(new_failures - old_failures),
                       'removed_failures': sorted(old_failures - new_failures)})
save('local-failure-comparison', comparison)

def findings(path):
    pattern = re.compile(r'^(.+?):\d+:\d+: (?:error|warning): (.+)$')
    return {(match[1].split('/source/', 1)[-1], match[2])
            for line in path.read_text().splitlines()
            if (match := pattern.match(line))}

before_lint = findings(origin.parent / 'artifacts/tc-main-targeted.log')
after_lint = findings(source / 'build/lint/targeted.log')
(artifacts / 'tc-main-targeted.log').write_bytes((source / 'build/lint/targeted.log').read_bytes())
lint = {'rechecked_units': ['src/tc/Main.cpp'], 'before': len(before_lint),
        'after': len(after_lint), 'added': sorted(after_lint - before_lint),
        'removed': sorted(before_lint - after_lint), 'full_lint_green': False,
        'baseline_or_suppression_increase': False}
save('lint-comparison', lint)
print(json.dumps({'source_sha256': identity['source_sha256'], 'generated': generated,
                  'ut': ut, 'local_failures': comparison, 'lint': lint}, indent=2))
