import importlib.util
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
build = source / 'build'
artifacts = root / 'artifacts'
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
before = verify.digest(source)
units = ('apps/base/system/visualization/page/CopyGenericChart.cpp',
         'apps/base/system/visualization/page/CopyGenericChart.def.cpp',
         'apps/base/system/visualization/codeunit/GenericChartMgt.cpp',
         'apps/base/system/visualization/page/GenericChartCustomization.cpp',
         'apps/base/system/visualization/page/GenericChartCustomization.def.cpp')
output = artifacts / 'actual-chart'
command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '--rtlib=compiler-rt',
           '--unwindlib=libunwind', '-fuse-ld=lld-19', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
           '-ffunction-sections', '-fdata-sections', '-I' + str(source / 'include'),
           '-I' + str(source / 'test/gate')]
command += ['-I' + str(source / 'apps' / app) for app in ('base', 'system', 'foundation', 'shared', 'absent')]
command += [str(root / 'ActualChart.cpp'), *[str(source / path) for path in units],
            '-L' + str(build), '-Wl,-rpath,' + str(build), '-Wl,--gc-sections',
            '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(output)]
compiled = subprocess.run(command, text=True, capture_output=True)
executed = subprocess.run([str(output)], text=True, capture_output=True) if compiled.returncode == 0 else None
after = verify.digest(source)
receipt = {'command': command, 'compile_exit': compiled.returncode, 'diagnostics': compiled.stderr,
           'exit': executed.returncode if executed else None,
           'stdout': executed.stdout if executed else None, 'stderr': executed.stderr if executed else None,
           'source_before': before, 'source_after': after, 'source_unchanged': before == after,
           'checks': 12, 'record_buffers_only': True, 'SQL_rendering_or_client_parity_proved': False}
(artifacts / 'actual-chart.json').write_text(json.dumps(receipt, indent=2) + '\n')
assert receipt['source_unchanged']
assert compiled.returncode == 0 and executed is not None and executed.returncode == 0
assert '12 check(s), 0 red' in executed.stdout
print(json.dumps({'compile_exit': compiled.returncode, 'stdout': executed.stdout,
                  'source_sha256': after}))
