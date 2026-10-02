from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
emitter_source = Path('/home/cosmo/Git/agiru/build/native-contract-20261001/artifacts/EmitContracts.cpp')
symbols = Path('/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH/src')
emitter = artifacts / 'emit-contracts'
command = ['clang++-19', '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
           '-I' + str(source / 'src/al'), '-I' + str(source / 'src/gen'),
           '-I' + str(source / 'include'), str(emitter_source),
           '-L' + str(source / 'build'), '-Wl,-rpath,' + str(source / 'build'),
           '-lagiru_gen', '-lagiru_al', '-o', str(emitter)]
compiled = subprocess.run(command, text=True, capture_output=True, timeout=60)
(artifacts / 'catalogue-emitter.log').write_text(compiled.stdout + compiled.stderr)
if compiled.returncode != 0:
    raise SystemExit(compiled.returncode)
emitted = subprocess.run([str(emitter), str(symbols), str(artifacts / 'contracts')],
                         text=True, capture_output=True, timeout=60)
(artifacts / 'catalogue-emission.log').write_text(emitted.stdout + emitted.stderr)
if emitted.returncode != 0:
    raise SystemExit(emitted.returncode)
for compiler in ('clang++-19', 'g++-14'):
    results = []
    for path in sorted((artifacts / 'contracts').glob('*.cpp')):
        command = [compiler, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                   '-fsyntax-only', '-I' + str(source / 'include'), str(path)]
        result = subprocess.run(command, text=True, capture_output=True, timeout=60)
        results.append({'table': path.stem, 'exit': result.returncode,
                        'diagnostics': result.stderr})
    receipt = {'compiler': compiler, 'candidates': len(results),
               'matching': sum(row['exit'] == 0 for row in results), 'results': results,
               'providers_or_execution_proved': False}
    (artifacts / f'contracts-{compiler}.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print(json.dumps({'compiler': compiler, 'candidates': len(results),
                     'matching': receipt['matching'],
                     'failed': [row['table'] for row in results if row['exit']]}), flush=True)
