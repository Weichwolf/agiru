from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
old = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
symbols = '/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH'
rows = []
for compiler, cxx in [('clang', 'clang++-19'), ('gcc', 'g++-14')]:
    build = old / ('build' if compiler == 'clang' else 'build/gcc')
    fixture = root / 'artifacts' / ('generated-old-' + compiler)
    (fixture / 'source').mkdir(parents=True, exist_ok=True)
    (fixture / 'apps.json').write_text(json.dumps({'apps': [{'name': 'fixture', 'source': 'source', 'depends': []}]}))
    (fixture / 'scope.json').write_text(json.dumps({'include': ['Microsoft.Fixture'], 'exclude': []}))
    for name in ('Option.Table.al', 'Option.Page.al'):
        (fixture / 'source' / name).write_bytes((root / name).read_bytes())
    generated = subprocess.run([str(build / 'agirutc'), str(fixture), str(fixture / 'apps.json'),
                                str(fixture / 'generated'), '--system-symbols', symbols],
                               text=True, capture_output=True, timeout=60)
    unit = fixture / 'generated/fixture/fixture/page/NativeOption.cpp'
    command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-fsyntax-only', '-I' + str(source / 'include'),
               '-I' + str(fixture / 'generated/fixture'),
               '-I' + str(fixture / 'generated/shared'), str(unit)]
    result = (subprocess.run(command, text=True, capture_output=True, timeout=90)
              if generated.returncode == 0 and unit.is_file() else None)
    valid = (result is not None and result.returncode != 0 and
             'OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701' in result.stderr)
    rows.append({'compiler': compiler, 'whole_generator_image': str(old),
                 'transpile_exit': generated.returncode, 'transpile_stdout': generated.stdout,
                 'transpile_stderr': generated.stderr, 'command': command,
                 'compile_exit': result.returncode if result else None,
                 'diagnostics': result.stderr if result else None, 'valid': valid})
(root / 'artifacts/generated-controls.json').write_text(json.dumps(rows, indent=2) + '\n')
print(json.dumps({'checks': len(rows), 'valid': all(row['valid'] for row in rows)}), flush=True)
raise SystemExit(0 if all(row['valid'] for row in rows) else 1)
