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
if verify.digest(origin) != identity['base_source_sha256']:
    raise RuntimeError('predecessor source image changed')
paths = ['include/platform/Field.h', 'include/type/Option.h', 'include/meta/EnumDef.h',
         'include/meta/TableDef.h', 'src/gen/TableWriter.cpp', 'src/gen/CodeunitWriter.cpp',
         'src/rt/written/PlatformField.cpp', 'test/gate/PlatformFieldGate.cpp',
         'test/gate/GenTableBindingGate.cpp']
old_files, new_files = set(verify.files(origin)), set(verify.files(source))
changed = sorted(str(path) for path in old_files & new_files
                 if (origin / path).read_bytes() != (source / path).read_bytes())
unexpected = [path for path in changed if path not in paths and not path.startswith('apps/')]
if old_files != new_files or unexpected:
    raise RuntimeError('unexpected source paths: ' + str(unexpected))
if (origin / 'test/slice').read_bytes() != (source / 'test/slice').read_bytes():
    raise RuntimeError('the compiling slice changed')
identity.update(source_sha256=verify.digest(source), base_digest_reverified_unchanged=True,
                production_integrated=False, prototype_paths=paths)
(root / 'source-identity.json').write_text(json.dumps(identity, indent=2) + '\n')

def save(name, value):
    (artifacts / f'{name}.json').write_text(json.dumps(value, indent=2) + '\n')

def read(name):
    return json.loads((artifacts / f'{name}.json').read_text())

def population(manifest):
    return {(entry['id'], entry['name'], method) for entry in manifest for method in entry['methods']}

before = read('ut-before-milestone.log.manifest')
after = read('ut-milestone.log.manifest')
old, new = population(before), population(after)
save('ut-population', {'before_codeunits': len(before), 'after_codeunits': len(after),
                       'before_methods': len(old), 'after_methods': len(new),
                       'missing': sorted(old - new), 'added': sorted(new - old),
                       'execution_proof': False})
save('generated-image', {'before_files': sum(path.startswith('apps/') for path in map(str, old_files)),
                         'after_files': sum(path.startswith('apps/') for path in map(str, new_files)),
                         'missing': [], 'added': [],
                         'changed': [path for path in changed if path.startswith('apps/')],
                         'translation_exit': read('transpile')['exit'],
                         'slice_byte_identical': True, 'full_al_compilation_proved': False,
                         'bcapps_main': subprocess.check_output(
                             ['git', '-C', '/home/cosmo/Git/BCApps', 'rev-parse', 'main'], text=True).strip()})
patch = []
for path in paths:
    patch.extend(difflib.unified_diff((origin / path).read_text().splitlines(keepends=True),
                                    (source / path).read_text().splitlines(keepends=True),
                                    fromfile='before/' + path, tofile='after/' + path))
(artifacts / 'prototype.patch').write_text(''.join(patch))

def failures(log):
    text = log.read_text()
    return set(re.findall(r'FAIL\s+an (?:unknown )?exception left (\S+)', text)) | set(
        re.findall(r'FAIL\s+[^\n]*/test/gate/([^/:]+)Gate\.cpp:', text))

rows = []
for compiler in ('clang', 'gcc'):
    previous = failures(origin.parent / 'artifacts' / f'test-{compiler}.log')
    current = failures(artifacts / f'test-{compiler}.log')
    rows.append({'compiler': compiler, 'before_failed_cases': len(previous),
                 'after_failed_cases': len(current), 'added_failures': sorted(current - previous),
                 'removed_failures': sorted(previous - current)})
save('local-failure-comparison', rows)
exits = [read(name) for name in ('tc', 'gcc', 'test-clang', 'test-gcc', 'transpile',
                               'transpile-repeat', 'ut', 'lint-field', 'lint-codeunit', 'lint-table')]
(root / 'proof-exits.json').write_text(json.dumps(exits, indent=2) + '\n')
print(json.dumps({'source_sha256': identity['source_sha256'], 'changed_source_paths': paths,
                  'changed_generated_files': len([path for path in changed if path.startswith('apps/')]),
                  'ut_methods': len(new), 'ut_codeunits': len(after), 'local_failures': rows}), flush=True)
