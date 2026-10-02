from pathlib import Path
import json
import subprocess
import sys

root = Path(__file__).resolve().parents[2]
proof = Path(__file__).resolve().parent
compiler = sys.argv[1] if len(sys.argv) > 1 else 'clang++-19'
results = []
for source in sorted((proof / 'contracts').glob('*.cpp')):
    result = subprocess.run([compiler, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic',
                             '-Werror', '-fsyntax-only', '-ferror-limit=0',
                             f'-I{root / "include"}', str(source)], text=True,
                            capture_output=True, timeout=120) if compiler.startswith('clang') else subprocess.run(
        [compiler, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-fsyntax-only',
         f'-I{root / "include"}', str(source)], text=True, capture_output=True, timeout=120)
    results.append({'table': source.stem, 'exit': result.returncode, 'diagnostics': result.stderr})
print(json.dumps({'compiler': compiler, 'candidates': len(results),
                  'matching': sum(row['exit'] == 0 for row in results),
                  'results': results}, indent=2))
sys.exit(0 if results and all(row['exit'] == 0 for row in results) else 1)
