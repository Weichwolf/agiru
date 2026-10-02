import hashlib
import json
from pathlib import Path
import re
import shlex
import subprocess

task = Path(__file__).resolve().parent
source = task / 'source'
artifacts = task / 'artifacts'
original = task.parent / 'compiler-llvm-20261001/system_symbols/src/Virtual Tables/PageTableField.Table.al'
text = original.read_text()
declaration = re.search(r'\btable\s+(\d+)\s+"([^"]+)"', text)
fields = list(re.finditer(r'\bfield\(\s*(\d+)\s*;\s*("[^"]+"|\w+)\s*;\s*(\w+)(?:\[(\d+)\])?\s*\)\s*\{([^}]+|)\}', text))
assert len(fields) == len(re.findall(r'\bfield\(', text)) == 15
tags = {'Integer': 7, 'Option': 5, 'Text': 31, 'Boolean': 3}
expected = [[declaration[1], declaration[2], '0', re.search(r'Scope\s*=\s*(\w+)', text)[1]]]
numbers = {}
for field in fields:
    number, name, kind, length, properties = field.groups()
    name = name.strip('"')
    numbers[name] = number
    codes = re.search(r'OptionOrdinalValues\s*=\s*([^;]+)', properties)
    obsolete = re.search(r'ObsoleteState\s*=\s*(\w+)', properties)
    reason = re.search(r"ObsoleteReason\s*=\s*'([^']*)'", properties)
    expected.append(['field', number, name, name, str(tags[kind]), length or '0',
                     codes[1].strip() if codes else '', obsolete[1] if obsolete else '',
                     reason[1] if reason else ''])
    members = re.search(r'OptionMembers\s*=\s*([^;]+)', properties)
    if members:
        values = [value.strip() for value in members[1].split(',')]
        ordinals = [value.strip() for value in codes[1].split(',')] if codes else list(map(str, range(len(values))))
        assert len(values) == len(ordinals)
        expected.extend(['option', number, ordinal, value, value] for ordinal, value in zip(ordinals, values))
keys = list(re.finditer(r'\bkey\(\s*(\w+)\s*;\s*([^)]*)\)', text))
assert len(keys) == 1
for key in keys:
    expected.append(['key', key[1], '1'] + [numbers[value.strip().strip('"')] for value in key[2].split(',')])
brick = re.search(r'fieldgroup\("Brick";\s*([^)]*)\)', text)
expected.append(['Brick'] + [numbers[value.strip()] for value in brick[1].split(',')])
flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
         '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19']
binary = artifacts / 'contract'
command = flags + ['-I' + str(source / 'include'), str(task / 'Contract.cpp'),
                   '-L' + str(source / 'build'), '-Wl,-rpath,' + str(source / 'build'),
                   '-lagiru_rt', '-lagiru_net', '-lagiru_al', '-lagiru_db', '-o', str(binary)]
subprocess.run(command, check=True)
result = subprocess.run([str(binary)], check=True, capture_output=True, text=True)
actual = [shlex.split(line) for line in result.stdout.splitlines()]
assert actual == expected, json.dumps({'expected': expected, 'actual': actual}, indent=2)
receipt = {'source': str(original), 'source_sha256': hashlib.sha256(original.read_bytes()).hexdigest(),
           'independent_of_AL_parser': True, 'expected': expected, 'actual': actual,
           'source_fields': 15, 'option_families': 4, 'option_values': 37,
           'live_provider_or_G1_proved': False}
(artifacts / 'source-contract.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'fields': 15, 'options': 37, 'contract_matches': True}))
