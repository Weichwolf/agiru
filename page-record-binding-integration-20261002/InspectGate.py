from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
source = task / 'source'
artifacts = task / 'artifacts'
text = (source / 'test/gate/GenSourceBindingGate.cpp').read_text()
needle = 'const auto body = agiru::gen::WriteSource(page, "Fixture.Page.al", objects, '
assert text.count(needle) == 2
lines = text.splitlines()
for at in reversed([i for i, line in enumerate(lines) if needle in line]):
    lines.insert(at + 1, '    std::printf("GENERATED BODY\\n%s\\n", body.c_str());')
probe = artifacts / 'InspectGate.cpp'
probe.write_text('\n'.join(lines) + '\n')
command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '--rtlib=compiler-rt',
    '--unwindlib=libunwind', '-fuse-ld=lld-19', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
    '-I' + str(source / 'include'), '-I' + str(source / 'src/gen'),
    '-I' + str(source / 'src/al'), '-I' + str(source / 'test/gate'), str(probe),
    '-L' + str(source / 'build'), '-Wl,-rpath,' + str(source / 'build'),
    '-lagiru_gen', '-lagiru_al', '-lagiru_rt', '-lagiru_net', '-lagiru_db',
    '-o', str(artifacts / 'InspectGate')]
subprocess.run(command, check=True)
result = subprocess.run([str(artifacts / 'InspectGate')], capture_output=True, text=True)
(artifacts / 'inspect-gate.log').write_text(result.stdout + result.stderr)
print(result.returncode)
