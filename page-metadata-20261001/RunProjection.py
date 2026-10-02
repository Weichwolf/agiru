from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parent
source = root / 'source'
compiler = sys.argv[1]
if compiler not in ('clang', 'gcc'):
    raise SystemExit('expected clang|gcc')
artifacts = root / 'artifacts'
mutant = artifacts / ('mutant-' + compiler)
header = source / 'include/platform/PageMetadata.h'
original = 'case PageType::UserControlHost: return std::nullopt;'
mutated = 'case PageType::UserControlHost: return PageMetadataPageType::Card;'
text = header.read_text()
if text.count(original) != 1:
    raise RuntimeError('missing unique unsupported projection match')
(mutant / 'platform').mkdir(parents=True, exist_ok=True)
(mutant / 'platform/PageMetadata.h').write_text(text.replace(original, mutated))
rows = []
for label, includes in [('current', source / 'include'), ('guessed-Card', mutant)]:
    executable = artifacts / ('projection-' + compiler + '-' + label)
    command = ['clang++-19' if compiler == 'clang' else 'g++-14', '-std=c++23',
               '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I' + str(includes),
               '-I' + str(source / 'include'), '-I' + str(source / 'test/gate'),
               str(root / 'ProjectionProbe.cpp'), '-o', str(executable)]
    compiled = subprocess.run(command, text=True, capture_output=True, timeout=90)
    executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=20)
                if compiled.returncode == 0 else None)
    rows.append({'image': label, 'compile_command': command, 'compile_exit': compiled.returncode,
                 'compile_stderr': compiled.stderr,
                 'execution_exit': executed.returncode if executed else None,
                 'stdout': executed.stdout if executed else None,
                 'stderr': executed.stderr if executed else None,
                 'no_libraries_or_database': True})
(artifacts / ('projection-' + compiler + '.json')).write_text(json.dumps(rows, indent=2) + '\n')
valid = all(row['compile_exit'] == 0 and row['execution_exit'] == (0 if row['image'] == 'current' else 1) and
            ('PageMetadataProjection: 20 check(s), ' + ('0 red' if row['image'] == 'current' else '6 red')) in row['stdout'] for row in rows)
print(json.dumps({'compiler': compiler, 'valid': valid, 'outputs': [row['stdout'] for row in rows]}), flush=True)
raise SystemExit(0 if valid else 1)
