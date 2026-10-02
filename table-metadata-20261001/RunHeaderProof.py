from pathlib import Path
import json
import os
import statistics
import subprocess
import sys
import time

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
artifacts = root / 'artifacts'
compiler = sys.argv[1]
if compiler not in ('clang', 'gcc'):
    raise SystemExit('expected clang|gcc')
cxx = 'clang++-19' if compiler == 'clang' else 'g++-14'
mutant = artifacts / ('reflection-no-option-' + compiler)
(mutant / 'platform').mkdir(parents=True, exist_ok=True)
text = (source / 'include/platform/ReflectionOptions.h').read_text()
match = '#include "type/Option.h"\n'
if text.count(match) != 1:
    raise RuntimeError('missing unique direct Option dependency')
(mutant / 'platform/ReflectionOptions.h').write_text(text.replace(match, ''))
rows = []
for label, headers in [('current', source / 'include'), ('missing-dependency', mutant)]:
    command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-fsyntax-only', '-I' + str(headers), '-I' + str(source / 'include'),
               str(root / 'ReflectionProbe.cpp')]
    checked = subprocess.run(command, text=True, capture_output=True, timeout=60)
    rows.append({'image': label, 'command': command, 'exit': checked.returncode,
                 'diagnostics': checked.stderr})
if rows[0]['exit'] != 0 or rows[1]['exit'] == 0 or 'OptionTraits' not in rows[1]['diagnostics']:
    raise RuntimeError('standalone reflection header control differs')
samples = []
for repeat in range(3):
    for label, image in [('before', origin), ('after', source)]:
        command = [cxx, '-std=c++23',
                   '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-fsyntax-only',
                   '-I' + str(image / 'include'), str(root / 'HeaderProbe.cpp')]
        started = time.monotonic()
        checked = subprocess.Popen(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        _, status, usage = os.wait4(checked.pid, 0)
        checked.returncode = os.waitstatus_to_exitcode(status)
        _, diagnostics = checked.communicate()
        if checked.returncode != 0:
            raise RuntimeError(diagnostics)
        samples.append({'image': label, 'repeat': repeat, 'elapsed_seconds': time.monotonic() - started,
                        'peak_rss_kib': usage.ru_maxrss, 'command': command,
                        'exit': checked.returncode})
receipt = {'compiler': compiler, 'standalone_checks': 4, 'controls': rows, 'samples': samples,
           'median': {label: {key: statistics.median(row[key] for row in samples if row['image'] == label)
                              for key in ('elapsed_seconds', 'peak_rss_kib')}
                      for label in ('before', 'after')},
           'full_build_or_erp_performance_proof': False, 'no_pch_or_ccache': True}
(artifacts / ('header-proof-' + compiler + '.json')).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'compiler': compiler, 'median': receipt['median'], 'controls_valid': True}), flush=True)
