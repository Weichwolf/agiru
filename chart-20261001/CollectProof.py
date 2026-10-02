from pathlib import Path
import importlib.util
import json
import re

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
identity = json.loads((root / 'source-identity.json').read_text())
origin = Path(identity['origin'])
spec = importlib.util.spec_from_file_location('verify_snapshot', source / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)

def read(name):
    return json.loads((artifacts / (name + '.json')).read_text())

def require(condition, message):
    if not condition:
        raise RuntimeError(message)

def tokens(text):
    return re.findall(r'"(?:[^"\\]|\\.)*"|\w+|[^\s]', text)

changes = read('changes')
require(verify.digest(origin) == identity['base_source_sha256'] == changes['predecessor_sha256'] and
        verify.digest(source) == changes['source_sha256'], 'source image changed')
require(changes['source_paths_added'] == ['include/platform/Chart.h'] and
        not changes['source_paths_removed'] and changes['generated_paths_unchanged'] and
        changes['slice_unchanged'], 'source scope or slice changed')
for filename, old_count, new_count in [('NativeObjectGate.cpp', 14, 15), ('GenTableBindingGate.cpp', 11, 13)]:
    def functions(owner):
        return dict(re.findall(r'^void (\w+)\(\) \{(.*?)^\}',
                    (owner / 'test/gate' / filename).read_text(), re.M | re.S))
    before, after = functions(origin), functions(source)
    require(len(before) == old_count and len(after) == new_count and
            all(name in after and tokens(body) == tokens(after[name]) for name, body in before.items()),
            'original gate functions changed: ' + filename)
require((origin / 'test/toolchain.py').read_bytes() == (source / 'test/toolchain.py').read_bytes(),
        'toolchain assertions changed')
def gate_tokens(owner):
    text = re.sub(r'^\s*//[^\n]*(?:\n|$)', '',
                  (owner / 'test/gate/GenCodeunitGate.cpp').read_text(), flags=re.M)
    return ''.join(tokens(text))

combined_owner = ''.join(tokens('Generated(source) + agiru::gen::WriteCodeunitSource('
    'agiru::al::ParseCodeunit(source), std::string(kAlPath), Tables())'))
restored_gate = gate_tokens(source)
require(restored_gate.count(combined_owner) == 1, 'codeunit gate owner change missing')
restored_gate = restored_gate.replace(combined_owner, ''.join(tokens('Generated(source)')))
require(restored_gate == gate_tokens(origin), 'codeunit assertions changed beyond dependency owner')
writer = Path('src/gen/CodeunitWriter.cpp')
def remove_once(text, before, after):
    require(text.count(before) == 1, 'anchored generator change missing')
    return text.replace(before, after)

restored = remove_once((source / writer).read_text(), '  add("Chart", "2000000078");\n', '')
restored = remove_once(restored, '''bool NeedsNativeDefinition(const TableRef &binding) {
  return Unprefixed(binding.identifier).starts_with("platform::");
}

''', '')
restored = remove_once(restored, '''      if (NeedsNativeDefinition(*ref)) {
        reachElement(ref->header);
      } else {
        ahead(ref->identifier);
      }''', '      ahead(ref->identifier);')
restored = remove_once(restored, '    both(procedure.returned);',
        '    for (const al::VarDecl &declared : procedure.variables) { both(declared); }\n'
        '    both(procedure.returned);')
require(tokens(restored) == tokens((origin / writer).read_text()),
        'generator changed beyond binding and native header ownership')
header = Path('src/gen/CodeunitWriter.h')
require(remove_once((source / header).read_text(),
        'bool NeedsNativeDefinition(const TableRef &binding);\n\n', '') ==
        (origin / header).read_text(), 'unexpected generator declaration change')
page = Path('src/gen/PageWriter.cpp')
require(remove_once((source / page).read_text(),
        'complete || declared.temporary || NeedsNativeDefinition(*ref)',
        'complete || declared.temporary') == (origin / page).read_text(),
        'unexpected page generator change')
door = Path('src/gen/Door.cpp')
restored_door = remove_once((source / door).read_text(), r'''  for (const std::string &header : headers) {
    const std::string directive = "#include \"" + header + "\"\n";
    if (!text.starts_with(directive) && text.find("\n" + directive) == std::string_view::npos) {
      out += directive;
    }
  }''', r'''  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }''')
require(tokens(restored_door) == tokens((origin / door).read_text()),
        'Door changed beyond duplicate owned directives')
registration = Path('src/rt/PlatformTables.cpp')
require((source / registration).read_text().replace('#include "platform/Chart.h"\n', '').replace(
        'const RegisterTable<platform::Chart> kChart;\n', '') == (origin / registration).read_text(),
        'runtime changed beyond native registration')
require((root / 'Chart.Codeunit.al').read_text().count('then Error(') == 21, 'AL guards changed')
oracle = read('al-oracle')
require(oracle['success'] and len(oracle['warnings']) == 2 and
        all(row[0] == 'AL0432' for row in oracle['warnings']), 'Pending warnings changed or suppressed')
def failures(path):
    log = path.read_text()
    return set(re.findall(r'FAIL\s+an (?:unknown )?exception left (\S+)', log)) | set(
        re.findall(r'FAIL\s+[^\n]*/test/gate/([^/:]+)Gate\.cpp:', log))
for compiler, name in [('clang', 'test'), ('gcc', 'test-gcc')]:
    before, after = failures(origin.parent / 'artifacts' / (name + '.log')), failures(artifacts / (name + '.log'))
    require(before == after and len(after) == 26, 'local failure identities changed')
    log = (artifacts / (name + '.log')).read_text()
    require(all(value in log for value in ('test: 92 case(s), 26 red', 'Ran 119 tests', '\nOK\n',
            'NativeObject: 1365 check(s), 0 red', 'GenTableBinding: 169 check(s), 0 red')),
            'local gate result missing')
    controls = read('controls-' + compiler)
    require(len(controls) == 4 and all(row['valid'] for row in controls), 'whole-image controls incomplete')
    require({(row['gate'], row['image']) for row in controls} ==
            {(gate, image) for gate in ('NativeObject', 'GenTableBinding')
                           for image in ('predecessor', 'current')}, 'control images missing')
    for row in controls:
        if row['image'] == 'predecessor' and row['gate'] == 'NativeObject':
            require(row['compile_exit'] != 0 and row['execution_exit'] is None and
                    'platform/Chart.h' in row['compile_stderr'], 'old native declaration absence hidden')
        else:
            total = 1365 if row['gate'] == 'NativeObject' else 169
            reds = 23 if row['image'] == 'predecessor' else 0
            require(row['compile_exit'] == 0 and f'{total} check(s), {reds} red' in row['stdout'],
                    'control assertion population differs')
    fixture = read('generated-' + compiler)
    require(all(fixture[key] == 0 for key in ('transpile_exit', 'compile_exit', 'execution_exit')) and
            '1 check(s), 0 red' in fixture['execution_stdout'], 'generated execution differs')
    contracts = read('contracts-' + compiler)
    require(contracts['candidates'] == 19 and contracts['matching'] == 18 and
            {row['table'] for row in contracts['results'] if row['exit'] != 0} == {'TenantLicenseState'},
            'source contracts differ')
for unit in ('codeunitwriter', 'pagewriter', 'door', 'platformtables'):
    lint = read('lint-' + unit + '-comparison')
    require(not lint['added'] and not lint['baseline_raised'], 'targeted findings grew')
repeat = read('repeat-output')
require(repeat['before_files'] == repeat['after_files'] == 24359 and
        not any(repeat[key] for key in ('missing', 'added', 'changed_bytes_or_mtime')), 'repeat differs')
for name, expected in [('native', 0), ('binder', 0), ('tc', 0), ('gcc', 0), ('test', 2),
                       ('test-gcc', 2), ('transpile', 0), ('transpile-repeat', 0)]:
    require(read(name)['exit'] == expected, 'unexpected make status: ' + name)
require(read('real-consumer')['success'], 'original consumer did not compile')
dependencies = read('dependencies')
require(dependencies['same_file_population'] and dependencies['redundancies_decreased'],
        'generated dependency baseline grew or population changed')
require(read('bodies')['success'] and not read('bodies')['unexpected'],
        'generated bodies changed outside the original Chart binding scope')
blast = read('header-blast-final')
require(blast['added_includes_with_no_named_type'] == 0,
        'new native header does not name its dependency')
native_headers = read('native-headers')
require(native_headers['success'] and len(native_headers['results']) == 4 and
        all(row['valid'] and row['syntax_exit'] == 0 and row['negative_exit'] != 0 and
            row['removed_owned_includes'] == 3 for row in native_headers['results']),
        'native header ownership negative controls missing')
require(read('codeunit-owner-control')['success'] and
        read('codeunit-owner-control')['same_37_checks_retained'],
        'local enum owner regression control missing')
cost = read('header-cost')
require(cost['no_pch'] and len(cost['samples']) == 8 and
        all(row['exit'] == 0 for row in cost['samples']), 'native registration cost probes incomplete')
actual = read('actual-chart')
require(len(actual['results']) == 2 and actual['expected_checks_per_compiler'] == 12,
        'actual consumer assertion population changed')
actual_success = actual['success'] and all(
    row['valid'] and row['stdout'] is not None and '12 check(s), 0 red' in row['stdout']
    for row in actual['results'])
if not actual_success:
    require(not actual['full_original_page_body_and_metadata_compiled_without_pch'] and
            not actual['executed_original_set_source_chart'] and
            actual['missing_checks_per_compiler'] == {'clang': 12, 'gcc': 12} and
            all(row['compile_exit'] != 0 and row['execution_exit'] is None and
                'DataMeasureType' in row['diagnostics'] for row in actual['results']),
            'unproved full consumer changed; investigate before retaining partial proof')
receipt = {**changes, 'original_native_gate_functions_token_identical': 14,
           'original_binder_gate_functions_token_identical': 11, 'native_checks': 1365,
           'predecessor_native_declaration_absent': True, 'predecessor_native_gate_not_executed': True,
           'binder_checks': 169, 'predecessor_binder_red': 23,
           'generated_al_guards': 21, 'generated_cpp_checks': 1, 'original_pending_warnings': 2,
           'actual_set_source_chart_checks_expected_per_compiler': 12,
           'actual_set_source_chart_checks_executed_per_compiler': 12 if actual_success else 0,
           'actual_set_source_chart_missing_per_compiler': 0 if actual_success else 12,
           'actual_full_consumer_success': actual_success, 'source_contracts': [18, 19],
           'local_cases': 92, 'unchanged_connection_failures': 26, 'toolchain_tests_unchanged': 119,
           'repeat_files': 24359, 'production_integrated': False, 'full_ut_pending': True,
           'redundant_directives_before': dependencies['results']['predecessor']['redundant_directives'],
           'redundant_directives_after': dependencies['results']['current']['redundant_directives'],
           'new_native_includes': blast['added_native_includes'],
           'generated_cpp_files_checked': read('bodies')['cpp_files'],
           'gen_codeunit_original_checks_retained': 37,
           'sealed_seed_activation_proof': False, 'g1_achieved': False,
           'sql_copy_rendering_or_client_parity_proof': False,
           'declaration_and_header_scope_proof_success': True,
           'preflight_success': actual_success}
name = 'preflight-proof.json' if actual_success else 'declaration-proof.json'
(artifacts / name).write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'preflight_success': actual_success, 'declaration_and_header_scope_proof_success': True,
                  'source_sha256': changes['source_sha256']}), flush=True)
raise SystemExit(0 if actual_success else 1)
