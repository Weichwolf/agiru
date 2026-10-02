from concurrent.futures import ThreadPoolExecutor
import importlib.util
import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
source = task / 'source'
artifacts = task / 'artifacts'
spec = importlib.util.spec_from_file_location('scope_inventory', source / 'scripts/scope_inventory.py')
inventory = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inventory)
symbols = task.parent / 'compiler-llvm-20261001/system_symbols'
raw = inventory.inventory(symbols, {'apps': [{'name': 'system', 'source': 'src'}]},
                          {'include': ['System'], 'exclude': [], 'product_exclude': []})
(artifacts / 'system-raw-inventory.json').write_text(json.dumps(raw, indent=2) + '\n')
assert not raw['errors'], raw['errors']
tables = [entry for entry in raw['objects'] if entry['kind'] == 'table']
assert len(tables) == 223 and raw['summary']['files'] == 362
compiler = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror']
driver = task / 'native-contracts'
command = compiler + [str(task / 'NativeContracts.cpp'), '-I' + str(source / 'src/al'),
    '-I' + str(source / 'src/gen'), '-I' + str(source / 'include'), '--rtlib=compiler-rt',
    '--unwindlib=libunwind', '-fuse-ld=lld-19', '-L' + str(source / 'build'),
    '-Wl,-rpath,' + str(source / 'build'), '-lagiru_gen', '-lagiru_al', '-o', str(driver)]
subprocess.run(command, check=True)

def probe(table):
    unit = artifacts / ('native-contract-' + str(table['id']) + '.cpp')
    emitted = subprocess.run([str(driver), str(symbols / table['source'])],
                             capture_output=True, text=True)
    result = dict(identity=table, source_parse_or_binding_exit=emitted.returncode,
                  source_parse_or_binding_error=emitted.stderr)
    if emitted.returncode != 0:
        return result
    unit.write_text(emitted.stdout)
    command = compiler + ['-fsyntax-only', '-ferror-limit=0', '-I' + str(source / 'include'), str(unit)]
    compiled = subprocess.run(command, capture_output=True, text=True)
    (unit.with_suffix('.log')).write_text(compiled.stdout + compiled.stderr)
    result.update(contract_unit=str(unit), compile_command=command, compile_exit=compiled.returncode,
                  diagnostics=compiled.stderr)
    return result

with ThreadPoolExecutor(max_workers=2) as pool:
    results = list(pool.map(probe, tables))
bound = [entry for entry in results if entry['source_parse_or_binding_exit'] == 0]
receipt = dict(raw_summary=raw['summary'], tables=len(tables), candidates=len(bound),
    source_parse_refused=[entry for entry in results if entry['source_parse_or_binding_exit'] not in (0, 3)],
    unbound=[entry for entry in results if entry['source_parse_or_binding_exit'] == 3],
    green=sum(entry['compile_exit'] == 0 for entry in bound),
    red=sum(entry['compile_exit'] != 0 for entry in bound),
    results=results, used_pch=False, G1_proved=False)
(artifacts / 'native-contracts.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({key: receipt[key] for key in ('tables', 'candidates', 'green', 'red')}))
print('Raw inventory: ' + json.dumps(raw['summary']))
assert not receipt['source_parse_refused'], receipt['source_parse_refused']
