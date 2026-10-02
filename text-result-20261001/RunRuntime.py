from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path('/home/cosmo/Git/agiru/build/native-catalogue-20261001/source')
artifacts = root / 'artifacts'
receipts = []

def execute(name, command):
    result = subprocess.run(command, text=True, capture_output=True, timeout=60)
    (artifacts / f'{name}.log').write_text(result.stdout + result.stderr)
    receipts.append({'name': name, 'command': command, 'exit': result.returncode})
    print(name, result.returncode, flush=True)
    return result

for compiler, build in (('clang++-19', 'build'), ('g++-14', 'build/gcc')):
    tag = 'clang' if compiler.startswith('clang') else 'gcc'
    binary = artifacts / f'text-old-{tag}'
    old_build = origin / build
    command = [compiler, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-I' + str(origin / 'include'), '-I' + str(source / 'test/gate'),
               str(source / 'test/gate/TextGate.cpp'), '-L' + str(old_build),
               '-Wl,-rpath,' + str(old_build), '-lagiru_rt', '-lagiru_net',
               '-lagiru_db', '-o', str(binary)]
    compiled = execute(f'text-old-compile-{tag}', command)
    if compiled.returncode == 0:
        execute(f'text-old-run-{tag}', [str(binary)])
    for label, headers in (('old', origin), ('current', source)):
        command = [compiler, '-std=c++23', '-O2', '-Wall', '-Wextra', '-Wpedantic',
                   '-Werror', '-fsyntax-only', '-I' + str(headers / 'include')]
        command += ['-I' + str(path) for path in (source / 'apps').iterdir() if path.is_dir()]
        command += [str(source / 'apps/base/accountant_portal/codeunit/InviteExternalAccountant.cpp')]
        execute(f'real-body-{label}-{tag}', command)
(artifacts / 'runtime-commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
