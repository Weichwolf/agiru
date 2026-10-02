from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
old = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
compiler = sys.argv[1]
if compiler not in ('clang', 'gcc'):
    raise SystemExit('expected clang|gcc')
cxx = 'clang++-19' if compiler == 'clang' else 'g++-14'
build = source / ('build' if compiler == 'clang' else 'build/gcc')
rows = []
driver = root / 'artifacts/TextGateDriver.cpp'
driver.write_text('#include "' + str(source / 'test/gate/TextGate.cpp') + '"\n'
                  'int PlainTextOverload(const agiru::Text<0> &value) { return Selected(value); }\n')
for name, headers, libraries in [('old', old / 'include', old / ('build' if compiler == 'clang' else 'build/gcc')),
                                  ('current', source / 'include', build),
                                  ('erased-result-mutant', root / 'artifacts/generated-mutant-include', build)]:
    output = root / 'artifacts' / ('text-' + compiler + '-' + name)
    command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-I' + str(headers),
               '-I' + str(source / 'test/gate'), str(driver),
               '-L' + str(libraries), '-Wl,-rpath,' + str(libraries),
               '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(output)]
    compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
    executed = (subprocess.run([str(output)], text=True, capture_output=True, timeout=30)
                if compiled.returncode == 0 else None)
    valid = (compiled.returncode != 0 and 'Guid.h:' in compiled.stderr and
             'return left + right.ToText();' in compiled.stderr if name == 'old' else
             compiled.returncode == 0 and executed is not None and
             executed.returncode == (0 if name == 'current' else 1) and
             ('Text: 75 check(s), ' + ('0' if name == 'current' else '6') + ' red') in executed.stdout)
    rows.append({'image': name, 'command': command, 'compile_exit': compiled.returncode,
                 'compile_stderr': compiled.stderr,
                 'execution_exit': executed.returncode if executed else None,
                 'stdout': executed.stdout if executed else None,
                 'stderr': executed.stderr if executed else None, 'valid': valid,
                 'mutant_changes_only_inline_result_type_and_conversion': name == 'erased-result-mutant'})
(root / 'artifacts' / ('gate-controls-' + compiler + '.json')).write_text(json.dumps(rows, indent=2) + '\n')
print(json.dumps({'compiler': compiler, 'valid': all(row['valid'] for row in rows),
                  'outputs': [row['stdout'] for row in rows]}), flush=True)
raise SystemExit(0 if all(row['valid'] for row in rows) else 1)
