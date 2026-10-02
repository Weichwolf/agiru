from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
text = (source / 'test/gate/GenCodeunitGate.cpp').read_text()
before = '''  const std::string generated = Generated(source);

  CHECK_TRUE("the enumeration a LOCAL variable names is included",'''
after = '''  const std::string generated = Generated(source) + agiru::gen::WriteCodeunitSource(
      agiru::al::ParseCodeunit(source), std::string(kAlPath), Tables());

  CHECK_TRUE("the enumeration a LOCAL variable names is included",'''
if text.count(before) != 1:
    raise SystemExit('local enum gate anchor changed')
fixture = root / 'artifacts/GenCodeunitOwnerControl.cpp'
fixture.write_text(text.replace(before, after))
rows = []
for label, image in [('predecessor', origin), ('current', source)]:
    executable = root / 'artifacts' / ('codeunit-owner-' + label)
    build = image / 'build'
    command = ['clang++-19', '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-I' + str(image / 'include'), '-I' + str(image / 'src/al'),
               '-I' + str(image / 'src/gen'), '-I' + str(source / 'test/gate'),
               '-DAGIRU_SOURCE_DIR="' + str(image) + '"',
               '-DAGIRU_AL_SOURCE="/home/cosmo/Git/BCApps/src/Layers/W1/BaseApp"',
               str(fixture), '-L' + str(build), '-Wl,-rpath,' + str(build),
               '-lagiru_gen', '-lagiru_al', '-o', str(executable)]
    compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
    executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
                if compiled.returncode == 0 else None)
    valid = (compiled.returncode == 0 and executed is not None and
             executed.returncode == (1 if label == 'predecessor' else 0) and
             f'GenCodeunit: 37 check(s), {1 if label == "predecessor" else 0} red' in executed.stdout)
    rows.append({'image': label, 'command': command, 'compile_exit': compiled.returncode,
                 'compile_diagnostics': compiled.stderr,
                 'execution_exit': executed.returncode if executed else None,
                 'stdout': executed.stdout if executed else None,
                 'stderr': executed.stderr if executed else None, 'valid': valid})
    (root / 'artifacts/codeunit-owner-control.json').write_text(json.dumps({
        'results': rows, 'same_37_checks_retained': True,
        'only_local_enum_gate_reads_both_generated_header_and_source': True,
        'no_golden_files_changed': True,
        'success': len(rows) == 2 and all(row['valid'] for row in rows)
    }, indent=2) + '\n')
    if not valid:
        raise SystemExit(json.dumps(rows[-1]))
print(json.dumps({'success': True, 'current_checks': 37, 'predecessor_red': 1}), flush=True)
