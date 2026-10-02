from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
origin = Path('/home/cosmo/Git/agiru/build/text-result-20261001/source')
unit = origin / 'src/al/Parser.cpp'
command = ['clang-tidy-19', '-p', str(origin / 'build'), '--quiet',
           '--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang',
           '--extra-arg=max-nodes=50000',
           '--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang',
           '--extra-arg=optin.performance.Padding:AllowedPad=64', str(unit)]
result = subprocess.run(command, text=True, capture_output=True, timeout=180)
(root / 'artifacts/parser-before-targeted.log').write_text(
    f'== src/al/Parser.cpp (exit {result.returncode}) ==\n' + result.stdout + result.stderr)
receipt = {'unit': str(unit), 'compiler_database': str(origin / 'build'),
           'exit': result.returncode, 'origin_modified': False,
           'reason': 'the prior 181-unit carried selection did not include Parser.cpp'}
(root / 'artifacts/parser-before-lint.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
