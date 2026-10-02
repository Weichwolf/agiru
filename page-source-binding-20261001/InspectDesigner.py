import hashlib
import json
from pathlib import Path
import sys

origin = Path('/home/cosmo/Git/agiru-worktrees/goal-20260928/build/field-native-proof')
sys.path.insert(0, str(origin / 'packages'))
import dnfile
from dncil.cil.body.reader import read_method_body_from_bytes

assembly = origin / 'Microsoft.Dynamics.Nav.Ncl.dll'
identity = json.loads((origin / 'runtime-identity.json').read_text())
assert assembly.stat().st_size == identity['bytes']
assert hashlib.sha256(assembly.read_bytes()).hexdigest() == identity['sha256']
pe = dnfile.dnPE(str(assembly))
types = []
for row in pe.net.mdtables.TypeDef.rows:
    qualified = str(row.TypeNamespace) + '.' + str(row.TypeName)
    if qualified not in ('Microsoft.Dynamics.Nav.Runtime.Designer.DesignerFieldType',
                          'Microsoft.Dynamics.Nav.Runtime.Designer.DesignerFieldProperty'):
        continue
    methods = []
    for method in row.MethodList:
        item = method.row
        assert str(item.Name).startswith('get_')
        assert item.Signature.value.hex() == '000008'
        body = read_method_body_from_bytes(pe.get_data(item.Rva))
        assert len(body.instructions) == 2 and body.instructions[-1].opcode.name == 'ret'
        first = body.instructions[0]
        opcode = first.opcode.name
        assert opcode.startswith('ldc.i4')
        value = first.operand if first.operand is not None else int(opcode.rsplit('.', 1)[1])
        methods.append({'name': str(item.Name)[4:], 'return': 'System.Int32', 'static': True,
                        'value': value, 'rva': item.Rva, 'signature': item.Signature.value.hex()})
    assert len(methods) == 10
    types.append({'name': qualified, 'properties': methods})
assert len(types) == 2
artifacts = Path(__file__).resolve().parent / 'artifacts'
artifacts.mkdir(exist_ok=True)
(artifacts / 'designer-contract.json').write_text(json.dumps({
    'original_assembly': str(assembly), 'verified_assembly_identity': identity,
    'types': types, 'live_designer_or_bc_workflow_proved': False}, indent=2) + '\n')
print(json.dumps({'types': len(types), 'properties': sum(len(row['properties']) for row in types),
                  'sha256': identity['sha256']}))
