import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import time

task = Path(__file__).resolve().parent
main = task.parent.parent
source = task / 'source'
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
counter_spec = importlib.util.spec_from_file_location('ut_manifest', source / 'scripts/ut_manifest.py')
counter = importlib.util.module_from_spec(counter_spec)
counter_spec.loader.exec_module(counter)
inputs = json.loads((task.parent / 'compiler-llvm-20261001/artifacts/inputs.json').read_text())
bc = task.parent / 'compiler-llvm-20261001/bc_source'
symbols = task.parent / 'compiler-llvm-20261001/system_symbols'
assert verify.digest(bc) == inputs['bc_source_sha256']
assert verify.digest(symbols) == inputs['system_symbols_sha256']

def files(tree, generated=False):
    if generated:
        return {str(path.relative_to(tree / 'apps')): hashlib.sha256(path.read_bytes()).hexdigest()
                for path in (tree / 'apps').rglob('*') if path.is_file()}
    return {str(relative): hashlib.sha256((tree / relative).read_bytes()).hexdigest()
            for relative in verify.files(tree) if relative.parts[0] != 'apps'}

before = verify.digest(source)
compiler_inputs = files(source)
old = files(main, True)
assert old == files(source, True) and len(old) == 24346
environment = dict(os.environ, CCACHE_DIR=str(task / 'ccache'), AGIRU_BC_SOURCE=str(bc),
    AGIRU_BC_REVISION=inputs['bc_revision'], AGIRU_SYSTEM_SYMBOLS=str(symbols))
command = ['make', 'transpile', 'JOBS=2']
started = time.monotonic()
with (artifacts / 'generation.log').open('w') as log:
    result = subprocess.run(command, cwd=source, env=environment, stdout=log, stderr=subprocess.STDOUT)
new = files(source, True)
entries = counter.scan(bc / 'Layers/W1/Tests')
methods = sorted((entry['id'], entry['name'], method) for entry in entries for method in entry['methods'])
whole_source_error = None
try:
    counter.scan(bc)
except ValueError as error:
    whole_source_error = str(error)
slice_before = (main / 'test/slice').read_bytes()
slice_after = (source / 'test/slice').read_bytes()
active = [line.strip() for line in slice_after.decode().splitlines()
          if line.strip() and not line.lstrip().startswith('#')]
receipt = dict(command=command, exit=result.returncode, elapsed_seconds=time.monotonic() - started,
    source_before=before, source_after=verify.digest(source),
    compiler_inputs_unchanged=compiler_inputs == files(source), frozen_inputs=inputs,
    generated_before=len(old), generated_after=len(new),
    added=sorted(new.keys() - old.keys()), removed=sorted(old.keys() - new.keys()),
    changed=sorted(path for path in old.keys() & new.keys() if old[path] != new[path]),
    generated_before_hashes=old, generated_after_hashes=new,
    scope_and_counter_unchanged=all((main / path).read_bytes() == (source / path).read_bytes()
        for path in ('scope.json', 'apps.json', 'scripts/ut_manifest.py')),
    slice_unchanged=slice_before == slice_after, active_slice_sources=len(active),
    missing_slice_sources=[path for path in active if not (source / 'apps' / path).is_file()],
    configured_UT_codeunits=len(entries), configured_UT_methods=len(methods), UT_identities=methods,
    AL_methods_executed=0, AL_methods_missing=len(methods),
    whole_source_UT_population_proved=False, whole_source_UT_probe_error=whole_source_error,
    compiler_system_source_consumption_implemented=False, G1_proved=False)
(artifacts / 'generation.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: value for key, value in receipt.items()
    if key not in ('generated_before_hashes', 'generated_after_hashes', 'UT_identities', 'changed')}), flush=True)
print('Changed generated files: ' + str(len(receipt['changed'])))
assert result.returncode == 0 and receipt['compiler_inputs_unchanged']
assert not receipt['added'] and not receipt['removed']
assert receipt['scope_and_counter_unchanged'] and receipt['slice_unchanged']
assert not receipt['missing_slice_sources'] and len(active) == 14212
assert len(entries) == 80 and len(methods) == 2310
