import hashlib
import json
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
symbols = root.parent / 'compiler-llvm-20261001/system_symbols'
expected = []
source_facts = []
for name in ('AllObj', 'AllObjWithCaption'):
    path = symbols / 'src/Virtual Tables' / (name + '.Table.al')
    text = path.read_text()
    declaration = re.search(r'\btable\s+(\d+)\s+(\w+)', text)
    assert declaration[2] == name
    matches = list(re.finditer(r'\bfield\(\s*(\d+)\s*;\s*"([^"]+)"\s*;\s*(\w+)(?:\[(\d+)\])?\s*\)', text))
    assert len(matches) == len(re.findall(r'\bfield\(', text)) == (6 if name == 'AllObj' else 9)
    members = re.search(r'OptionMembers\s*=\s*([^;]+);', text)[1].split(',')
    members = [value.strip().strip('"') for value in members]
    captions = re.search(r"OptionCaption\s*=\s*'([^']*)'", text)[1].split(',')
    assert len(members) == len(captions) == 23
    fields = [{'number': int(match[1]), 'name': match[2], 'caption': match[2],
               'type': match[3].capitalize(), 'length': int(match[4] or '0'),
               'options': [{'ordinal': index, 'name': value, 'caption': captions[index]}
                           for index, value in enumerate(members)] if match[3].lower() == 'option' else []}
              for match in matches]
    numbers = {field['name']: field['number'] for field in fields}
    keys = list(re.finditer(r'\bkey\(\s*(\w+)\s*;\s*([^)]*)\)', text))
    assert len(keys) == 1
    key = keys[0]
    raw_key_fields = [value.strip().strip('"') for value in key[2].split(',')]
    expected.append({'id': int(declaration[1]), 'name': name,
                     'data_per_company': re.search(r'DataPerCompany\s*=\s*(\w+)', text)[1].lower() == 'true',
                     'inherent_permissions': re.search(r'InherentPermissions\s*=\s*(\w+)', text)[1],
                     'fields': fields,
                     'keys': [{'name': key[1], 'fields': [numbers[value] for value in raw_key_fields],
                               'clustered': True}]})
    source_facts.append({'source': str(path), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                         'scope': re.search(r'Scope\s*=\s*(\w+)', text)[1],
                         'field_groups': re.findall(r'fieldgroup\(\s*"([^"]+)"\s*;\s*([^)]*)\)', text)})
rows = []
for label, source in [('before', root.parent.parent), ('after', root / 'source')]:
    libraries = source / 'build'
    binary = artifacts / ('contract-' + label)
    command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
               '-I' + str(source / 'include'), str(root / 'Contract.cpp'), '-L' + str(libraries),
               '-Wl,-rpath,' + str(libraries), '-lagiru_rt', '-lagiru_net', '-lagiru_al', '-lagiru_db',
               '-o', str(binary)]
    compiled = subprocess.run(command, capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    actual = json.loads(result.stdout)
    differences = [table['name'] for table, native in zip(expected, actual) if table != native]
    assert len(actual) == len(expected) == 2
    assert bool(differences) == (label == 'before')
    rows.append({'label': label, 'actual': actual, 'mismatched_tables': differences})
receipt = {'source_facts': source_facts, 'independent_of_AL_parser': True, 'expected': expected, 'rows': rows,
           'retained_gaps': ['common Scope/fieldgroup representation', 'read-only live providers',
                            'installed app/package provenance', 'populated schema activation'],
           'runtime_provider_SQL_or_G1_proved': False}
(artifacts / 'source-contract.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'source_fields': 15, 'source_option_positions': 23,
                  'before_mismatched_tables': rows[0]['mismatched_tables'], 'after_mismatched_tables': rows[1]['mismatched_tables']}))
