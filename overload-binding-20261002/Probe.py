import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys

task = Path(__file__).resolve().parent
main = task.parent.parent
artifacts = task / 'artifacts' / (sys.argv[1] if len(sys.argv) > 1 else 'checked')
artifacts.mkdir(exist_ok=True)
spec = importlib.util.spec_from_file_location('verify_snapshot', main / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
identity = verify.digest(main)
compiler = task.parent / 'text-guid-20261001/tools/al/alc'
symbols = Path('/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH')
rows = []
for name in ('arity', 'type', 'reference', 'literal'):
    project = artifacts / name
    project.mkdir()
    shutil.copy2(task / 'app.json', project / 'app.json')
    source = task / name / 'fixture/Probe.Codeunit.al'
    shutil.copy2(source, project / 'Probe.Codeunit.al')
    command = [str(compiler), '/project:' + str(project), '/packagecachepath:' + str(symbols),
               '/out:' + str(project / 'Probe.app'), '/warnaserror+', '/parallel-']
    oracle = subprocess.run(command, capture_output=True, text=True, timeout=60)
    (project / 'oracle.log').write_text(oracle.stdout + oracle.stderr)
    assert oracle.returncode == 0, oracle.stdout + oracle.stderr
    generated = project / 'generated'
    emit = subprocess.run([str(main / 'build/agirutc'), str(task / name),
        str(task / 'apps.json'), str(generated)], capture_output=True, text=True, timeout=60)
    (project / 'generation.log').write_text(emit.stdout + emit.stderr)
    assert emit.returncode == 0, emit.stdout + emit.stderr
    flags = ['clang++-19', '-x', 'c++', '-std=c++23', '-stdlib=libc++', '--rtlib=compiler-rt',
        '--unwindlib=libunwind', '-fuse-ld=lld-19', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
        '-I' + str(main / 'include'), '-I' + str(generated), '-I' + str(generated / 'fixture'),
        '-I' + str(generated / 'shared'), '-I' + str(generated / 'absent')]
    executable = project / 'consumer'
    command = flags + [str(path) for path in sorted(generated.rglob('*.cpp'))]
    command += [str(task / 'Consumer.cpp.in'), '-L' + str(main / 'build'),
        '-Wl,-rpath,' + str(main / 'build'), '-lagiru_rt', '-lagiru_al', '-lagiru_net', '-lagiru_db',
        '-o', str(executable)]
    compiled = subprocess.run(command, capture_output=True, text=True, timeout=90)
    (project / 'compile.log').write_text(compiled.stdout + compiled.stderr)
    executed = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10) if compiled.returncode == 0 else None
    if executed:
        (project / 'execution.log').write_text(executed.stdout + executed.stderr)
    rows.append({'name': name, 'AL_source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
        'oracle_exit': oracle.returncode, 'generation_exit': emit.returncode,
        'compile_exit': compiled.returncode, 'execution_exit': executed.returncode if executed else None})
    print(json.dumps(rows[-1]), flush=True)
receipt = dict(main_source_sha256=identity, source_unchanged=verify.digest(main) == identity,
    compiler_assembly_sha256=hashlib.sha256((compiler.parent / 'Microsoft.Dynamics.Nav.CodeAnalysis.dll').read_bytes()).hexdigest(),
    system_app_sha256=hashlib.sha256((symbols / 'System.app').read_bytes()).hexdigest(),
    rows=rows, uses_database=False, CPP_fix_implemented=False, BC_runtime_executed=False)
(artifacts / 'probe.json').write_text(json.dumps(receipt, indent=2) + '\n')
assert receipt['source_unchanged']
assert all(row['compile_exit'] != 0 or row['execution_exit'] != 0 for row in rows)
print(json.dumps(receipt))
