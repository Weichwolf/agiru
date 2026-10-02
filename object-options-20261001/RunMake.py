from pathlib import Path
import json
import hashlib
import importlib.util
import os
import subprocess
import sys
import time

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
artifacts.mkdir(exist_ok=True)
name, *arguments = sys.argv[1:]
environment = dict(os.environ, CCACHE_DIR='/tmp/agiru-native-20261001.WC8RiY/ccache',
                   AGIRU_BC_SOURCE='/home/cosmo/Git/BCApps/src',
                   AGIRU_SYSTEM_SYMBOLS='/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH')
jobs = os.environ.get('AGIRU_PROOF_JOBS', '2')
if jobs not in ('2', '6'):
    raise SystemExit('expected two local or six serialized integration jobs')
command = ['make', *arguments, 'JOBS=' + jobs]
def image():
    return {str(path.relative_to(root / 'source/apps')):
            (hashlib.sha256(path.read_bytes()).hexdigest(), path.stat().st_mtime_ns)
            for path in (root / 'source/apps').rglob('*') if path.is_file()}
before_image = image() if name == 'transpile-repeat' else {}
verify = None
before_source = None
if 'ut' in arguments:
    spec = importlib.util.spec_from_file_location('verify_snapshot', root / 'source/scripts/verify_snapshot.py')
    verify = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(verify)
    before_source = verify.digest(root / 'source')
    (artifacts / 'source-before-ut.json').write_text(json.dumps({'source_sha256': before_source}) + '\n')
cache_before = subprocess.run(['ccache', '--show-stats', '--format=json'], env=environment,
                              text=True, capture_output=True)
if 'ut' in arguments:
    command.append('UT_LOG=' + str(artifacts / 'ut-milestone.log'))
started = time.monotonic()
with (artifacts / (name + '.log')).open('w') as log:
    result = subprocess.run(command, cwd=root / 'source', env=environment,
                            stdout=log, stderr=subprocess.STDOUT)
receipt = {'command': command, 'exit': result.returncode,
           'elapsed_seconds': time.monotonic() - started}
if verify is not None:
    receipt['source_before'] = before_source
    receipt['source_after'] = verify.digest(root / 'source')
    receipt['source_unchanged_during_build'] = receipt['source_before'] == receipt['source_after']
    if not receipt['source_unchanged_during_build']:
        raise RuntimeError('UT source image changed during the build')
cache_after = subprocess.run(['ccache', '--show-stats', '--format=json'], env=environment,
                             text=True, capture_output=True)
receipt['cache_before'] = cache_before.stdout
receipt['cache_after'] = cache_after.stdout
(artifacts / (name + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
if name == 'transpile-repeat':
    after_image = image()
    repeat = {'before_files': len(before_image), 'after_files': len(after_image),
              'missing': sorted(before_image.keys() - after_image.keys()),
              'added': sorted(after_image.keys() - before_image.keys()),
              'changed_bytes_or_mtime': sorted(path for path in before_image.keys() & after_image.keys()
                                               if before_image[path] != after_image[path])}
    (artifacts / 'repeat-output.json').write_text(json.dumps(repeat, indent=2) + '\n')
if 'lint-one' in arguments:
    for suffix in ('targeted.log', 'targeted-units.json'):
        (artifacts / (name + '-' + suffix)).write_bytes((root / 'source/build/lint' / suffix).read_bytes())
print(json.dumps(receipt), flush=True)
raise SystemExit(result.returncode)
