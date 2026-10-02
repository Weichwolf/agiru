from pathlib import Path
import hashlib
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
old = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
relative = Path('apps/base/system/environment/configuration/page/ReportSettings.cpp')
if (old / relative).read_bytes() != (source / relative).read_bytes():
    raise RuntimeError('the next failing consumer changed')
rows = []
for compiler, cxx in [('clang', 'clang++-19'), ('gcc', 'g++-14')]:
    for label, image in [('predecessor', old), ('current', source)]:
        command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                   '-fsyntax-only', '-I' + str(image / 'include')]
        command += ['-I' + str(image / 'apps' / app) for app in
                    ('base', 'system', 'foundation', 'shared', 'absent')]
        command.append(str(image / relative))
        result = subprocess.run(command, text=True, capture_output=True, timeout=90)
        valid = (result.returncode != 0 and 'Refused' in result.stderr and
                 'Variable = T{};' in result.stderr and 'ReportSettings.cpp:340' in result.stderr)
        rows.append({'compiler': compiler, 'image': label, 'command': command,
                     'exit': result.returncode, 'diagnostics': result.stderr, 'valid': valid})
receipt = {'results': rows, 'success': all(row['valid'] for row in rows),
           'consumer_unchanged': True,
           'consumer_sha256': hashlib.sha256((source / relative).read_bytes()).hexdigest(),
           'no_pch': True, 'diagnostic_only': True, 'cause_fixed': False,
           'binding_conflict': {'table': 'Object Options', 'source_id': 2000000196,
                                'intrinsic_id': 2000000225},
           'do_not_add_default_refused_construction_or_clear_noop': True}
(root / 'artifacts/next-diagnostic.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'checks': len(rows), 'success': receipt['success'], 'fixed': False}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
