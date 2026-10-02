import json
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
receipt = json.loads((artifacts / 'test-2.json').read_text())
log = (artifacts / 'test-2.log').read_text()
gate_sources = sorted((source / 'test/gate').glob('*.cpp'))
assert len(gate_sources) == 86
assert 'test: 89 case(s), 26 red' in log
assert re.search(r'Ran 104 tests in [0-9.]+s\s+OK\b', log)
assert receipt['source_unchanged'] and receipt['exit'] == 2
cases = []
for path in gate_sources:
    result = subprocess.run([str(source / 'build' / ('gate_' + path.stem))],
                            cwd=source, text=True, capture_output=True, timeout=30)
    cases.append({'gate': path.stem, 'exit': result.returncode,
                  'output': result.stdout + result.stderr})
failures = [case for case in cases if case['exit']]
assert len(failures) == 26
assert all('Connection:' in case['output'] for case in failures)
libraries = {}
for path in sorted((source / 'build').glob('libagiru_*.so')):
    dynamic = subprocess.run(['readelf', '-d', str(path)], capture_output=True, text=True, check=True)
    needed = re.findall(r'Shared library: \[([^\]]+)\]', dynamic.stdout)
    assert 'libc++.so.1' in needed
    assert 'libstdc++.so.6' not in needed and 'libgcc_s.so.1' not in needed
    libraries[path.name] = needed
assert len(libraries) == 5
config = (source / 'build/build.ninja').read_text()
for flag in ('-stdlib=libc++', '--rtlib=compiler-rt', '--unwindlib=libunwind',
             '-fuse-ld=/usr/bin/ld.lld-19'):
    assert flag in config
assert 'AGIRU_LLVM_CXX23_WORKS:INTERNAL=1' in (source / 'build/CMakeCache.txt').read_text()
proof = {'source_sha256': receipt['source_after'], 'source_unchanged': True,
         'core_and_gates_built': True, 'local_population': 89, 'cpp_population': 86,
         'local_failed': 26, 'toolchain_population': 104, 'toolchain_failed': 0,
         'toolchain_skipped': 0, 'failed_cpp_cases': [case['gate'] for case in failures],
         'cpp_results': cases, 'direct_library_dependencies': libraries,
         'cxx23_compile_link_probe_passed': True,
         'scope': 'Main-source own LLVM core/gate build. No slice/full-app/UT, production integration or ERP-completeness claim.'}
(artifacts / 'proof.json').write_text(json.dumps(proof, indent=2) + '\n')
print(json.dumps({key: value for key, value in proof.items() if key != 'cpp_results'}, indent=2))
