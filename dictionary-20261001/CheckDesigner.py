import hashlib
import json
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
artifacts.mkdir(exist_ok=True)
contract = json.loads((root.parent / 'page-source-binding-20261001/artifacts/designer-contract.json').read_text())
assembly = Path(contract['original_assembly'])
assert hashlib.sha256(assembly.read_bytes()).hexdigest() == contract['verified_assembly_identity']['sha256']
assert assembly.stat().st_size == contract['verified_assembly_identity']['bytes']
program = source / 'test/gate/DesignerConstantsGate.cpp'
libraries = source / 'build'
flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
         '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
         '-I' + str(source / 'include'), '-I' + str(source / 'test/gate')]
link = ['-L' + str(libraries), '-Wl,-rpath,' + str(libraries), '-lagiru_net', '-lagiru_al']
target = artifacts / 'designer-current'
command = [*flags, str(program), *link, '-o', str(target)]
compiled = subprocess.run(command, capture_output=True, text=True)
assert compiled.returncode == 0, compiled.stderr
current = subprocess.run([str(target)], capture_output=True, text=True)
assert current.returncode == 0, current.stdout + current.stderr
print(current.stdout)
facts = []
mutants = []
for family in contract['types']:
    name = family['name'].rsplit('.', 1)[1]
    header = source / 'include/dotnet' / (name + '.h')
    text = header.read_text()
    constants = list(re.finditer(r'static constexpr ::agiru::Integer k(\w+) = (\d+);', text))
    signatures = list(re.finditer(r'static constexpr ::agiru::Integer (\w+)\(\)\s*\{\s*return k(\w+);\s*\}', text))
    assert all(match[1] == match[2] for match in signatures)
    actual = {match[1]: int(match[2]) for match in constants}
    expected = {row['name']: row['value'] for row in family['properties']}
    assert actual == expected and len(signatures) == len(constants) == 10
    assert {match[1] for match in signatures} == set(actual)
    for match in constants:
        facts.append({'family': name, 'getter': match[1], 'value': int(match[2]), 'return': 'System.Int32'})
        changed = text[:match.start(2)] + str(int(match[2]) + 1) + text[match.end(2):]
        fixture = artifacts / 'mutants' / (name + '-' + match[1])
        (fixture / 'dotnet').mkdir(parents=True, exist_ok=True)
        (fixture / 'dotnet' / header.name).write_text(changed)
        binary = fixture / 'gate'
        command = [*flags[:9], '-I' + str(fixture), *flags[9:], str(program), *link, '-o', str(binary)]
        built = subprocess.run(command, capture_output=True, text=True)
        executed = None
        if built.returncode == 0:
            executed = subprocess.run([str(binary)], capture_output=True, text=True)
            assert executed.returncode != 0, f'mutant escaped: {name}.{match[1]}'
        else:
            assert 'static assertion failed' in built.stderr, built.stderr
        mutants.append({'family': name, 'getter': match[1], 'compile_exit': built.returncode,
                        'execution_exit': executed.returncode if executed else None,
                        'diagnostics': built.stderr if executed is None else executed.stdout + executed.stderr})
receipt = {'original_assembly_sha256': contract['verified_assembly_identity']['sha256'],
           'complete_original_getters': facts, 'changed_value_controls': mutants,
           'all_twenty_constants_match_original': True,
           'current_exit': current.returncode, 'current_stdout': current.stdout,
           'live_designer_or_BC_workflow_proved': False}
(artifacts / 'designer-proof.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'constants': len(facts), 'rejected_mutants': len(mutants)}))
