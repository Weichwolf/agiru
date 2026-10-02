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
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
generation = json.loads((artifacts / 'generation.json').read_text())

def files(tree, generated=False):
    if generated:
        return {str(path.relative_to(tree / 'apps')): hashlib.sha256(path.read_bytes()).hexdigest()
                for path in (tree / 'apps').rglob('*') if path.is_file()}
    return {str(relative): hashlib.sha256((tree / relative).read_bytes()).hexdigest()
            for relative in verify.files(tree) if relative.parts[0] != 'apps'}

assert files(main) == files(source)
assert files(main, True) == generation['generated_before_hashes']
inputs = files(main)
before = verify.digest(main)
bc = task.parent / 'compiler-llvm-20261001/bc_source'
environment = dict(os.environ, CCACHE_DIR=str(task / 'ccache'), AGIRU_BC_SOURCE=str(bc))
environment.pop('AGIRU_SYSTEM_SYMBOLS', None)
command = ['make', 'transpile', 'JOBS=2']
started = time.monotonic()
with (artifacts / 'main-generation.log').open('w') as log:
    result = subprocess.run(command, cwd=main, env=environment, stdout=log, stderr=subprocess.STDOUT)
actual = files(main, True)
receipt = dict(command=command, exit=result.returncode, elapsed_seconds=time.monotonic() - started,
    source_before=before, source_after=verify.digest(main), compiler_inputs_unchanged=inputs == files(main),
    expected_generated_files=len(generation['generated_after_hashes']), actual_generated_files=len(actual),
    generated_matches_reviewed_image=actual == generation['generated_after_hashes'],
    source_matches_reviewed_image=verify.digest(main) == verify.digest(source), G1_proved=False)
(artifacts / 'main-generation.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
assert result.returncode == 0 and receipt['compiler_inputs_unchanged']
assert receipt['generated_matches_reviewed_image'] and receipt['source_matches_reviewed_image']
