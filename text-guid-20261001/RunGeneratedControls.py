from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
old = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
rows = []
for compiler, cxx in [('clang', 'clang++-19'), ('gcc', 'g++-14')]:
    generated = root / 'artifacts' / ('generated-' + compiler) / 'generated'
    units = list(generated.rglob('*.cpp'))
    if len(units) != 1:
        raise RuntimeError('generated fixture population differs')
    for name, headers, image in [('old', old / 'include', old),
                                 ('erased-result-mutant', root / 'artifacts/generated-mutant-include', source)]:
        build = image / ('build' if compiler == 'clang' else 'build/gcc')
        command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                   '-I' + str(headers),
                   '-I' + str(source / 'test/gate'), '-I' + str(generated / 'fixture'),
                   '-I' + str(generated / 'shared'), str(root / 'GeneratedJoin.cpp'), *map(str, units),
                   '-L' + str(build), '-Wl,-rpath,' + str(build),
                   '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o',
                   str(root / 'artifacts' / ('generated-control-' + compiler + '-' + name))]
        result = subprocess.run(command, text=True, capture_output=True, timeout=120)
        valid = result.returncode != 0 and 'redefinition' not in result.stderr and ('Guid.h:' in result.stderr and
                    'return left + right.ToText();' in result.stderr if name == 'old' else
                    'ambiguous' in result.stderr and 'Which' in result.stderr)
        rows.append({'compiler': compiler, 'image': name, 'command': command,
                     'exit': result.returncode, 'diagnostics': result.stderr, 'valid': valid,
                     'execution_attempted': False})
receipt = {'results': rows, 'success': all(row['valid'] for row in rows)}
(root / 'artifacts/generated-controls.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'checks': len(rows), 'success': receipt['success']}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
