from pathlib import Path
import subprocess
import sys

task = Path(__file__).resolve().parent
prefix = sys.argv[1] + '-' if len(sys.argv) > 1 else ''
for label, tree in [('before', task.parent.parent), ('after', task / 'source')]:
    executable = task / 'artifacts' / ('InspectReport-' + label)
    command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '--rtlib=compiler-rt',
        '--unwindlib=libunwind', '-fuse-ld=lld-19', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
        '-I' + str(tree / 'include'), '-I' + str(tree / 'src/gen'), '-I' + str(tree / 'src/al'),
        str(task / 'artifacts/InspectReport.cpp'), '-L' + str(tree / 'build'),
        '-Wl,-rpath,' + str(tree / 'build'), '-lagiru_gen', '-lagiru_al', '-o', str(executable)]
    subprocess.run(command, check=True)
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    (task / 'artifacts' / (prefix + 'inspect-report-' + label + '.log')).write_text(result.stdout + result.stderr)
    print(label, result.returncode)
