import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
parent = root.parent / 'compiler-llvm-20261001'
artifacts = root / 'artifacts'
flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror']
link = ['--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19']
rows = []
for label, libraries in [('old', parent / 'source/build'), ('new', source / 'build')]:
    target = artifacts / ('binder-' + label)
    command = flags + link + ['-I' + str(source / 'src/al'), '-I' + str(source / 'src/gen'),
                             '-I' + str(source / 'include'), '-I' + str(source / 'test/gate'),
                             str(source / 'test/gate/GenTableBindingGate.cpp'),
                             '-L' + str(libraries), '-Wl,-rpath,' + str(libraries),
                             '-lagiru_gen', '-lagiru_al', '-o', str(target)]
    compiled = subprocess.run(command, capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    executed = subprocess.run([str(target)], capture_output=True, text=True)
    rows.append({'label': label, 'command': command, 'exit': executed.returncode,
                 'stdout': executed.stdout, 'stderr': executed.stderr})
assert rows[0]['exit'] == 1 and 'Page Table Field' in rows[0]['stdout']
assert rows[1]['exit'] == 0
target = artifacts / 'emit-contract'
command = flags + link + ['-I' + str(source / 'src/al'), '-I' + str(source / 'src/gen'),
                         '-I' + str(source / 'include'), str(root / 'EmitContract.cpp'),
                         '-L' + str(source / 'build'), '-Wl,-rpath,' + str(source / 'build'),
                         '-lagiru_gen', '-lagiru_al', '-o', str(target)]
subprocess.run(command, check=True)
emitted = subprocess.run([str(target), str(parent / 'system_symbols/src/Virtual Tables/PageTableField.Table.al')],
                         check=True, capture_output=True, text=True).stdout
syntax = flags + ['-fsyntax-only', '-I' + str(source / 'include')]
variants = [('source', emitted),
            ('wrong-id', emitted.replace('TableId{2000000171}', 'TableId{2000000172}')),
            ('wrong-caption-length', emitted.replace('field->length != 80', 'field->length != 79')),
            ('wrong-native-type-name', emitted.replace('NotSupported_Binary', 'Binary')),
            ('wrong-native-code', emitted.replace('ordinal != 31488', 'ordinal != 9')),
            ('wrong-key-order', emitted.replace('fields[0] == ::agiru::FieldNo{1}',
                                                 'fields[0] == ::agiru::FieldNo{2}'))]
contracts = []
for label, text in variants:
    assert label == 'source' or text != emitted, label
    path = artifacts / ('contract-' + label + '.cpp')
    path.write_text(text)
    result = subprocess.run(syntax + [str(path)], capture_output=True, text=True)
    contracts.append({'label': label, 'exit': result.returncode, 'diagnostics': result.stderr})
assert contracts[0]['exit'] == 0, contracts[0]['diagnostics']
assert all(row['exit'] != 0 and 'static assertion failed' in row['diagnostics']
           for row in contracts[1:]), contracts
(artifacts / 'source-controls.json').write_text(json.dumps({'binder': rows, 'contracts': contracts}, indent=2) + '\n')
print(json.dumps({'binder_exits': [row['exit'] for row in rows],
                  'contract_exits': [row['exit'] for row in contracts]}))
