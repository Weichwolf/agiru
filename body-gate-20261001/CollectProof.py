from pathlib import Path
import hashlib
import importlib.util
import json
import shutil

root = Path(__file__).resolve().parent
source = root / 'source'
repository = root.parents[1]
origin = Path('/home/cosmo/Git/agiru/build/native-catalogue-20261001/source')
artifacts = root / 'artifacts'

def module(name, relative):
    spec = importlib.util.spec_from_file_location(name, source / relative)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result

verify = module('verify_snapshot', 'scripts/verify_snapshot.py')
manifest = module('ut_manifest', 'scripts/ut_manifest.py')
identity = json.loads((root / 'source-identity.json').read_text())
before = verify.digest(origin)
if before != identity['base_source_sha256']:
    raise RuntimeError('predecessor image changed')
identity.update(source_sha256=verify.digest(source), base_digest_reverified_unchanged=True,
                integrated_paths=['Makefile::gap', 'scripts/first_gap.sh',
                                  'test/toolchain.py::FirstGapGate'],
                native_prototypes_integrated=False)
(root / 'source-identity.json').write_text(json.dumps(identity, indent=2) + '\n')

def population(entries):
    return {(entry['id'], entry['name'], method)
            for entry in entries for method in entry['methods']}

snapshot = repository / 'build/verify/20260930T210740Z-797012'
old = json.loads((snapshot / 'artifacts/ut.log.manifest.json').read_text())
new = manifest.scan(Path('/home/cosmo/Git/BCApps/src/Layers/W1/Tests'))
old_ids, new_ids = population(old), population(new)
ut = {'snapshot': snapshot.name, 'before_codeunits': len(old), 'after_codeunits': len(new),
      'before_methods': len(old_ids), 'after_methods': len(new_ids),
      'missing': sorted(old_ids - new_ids), 'added': sorted(new_ids - old_ids),
      'execution_proof': False}
(artifacts / 'ut-population.json').write_text(json.dumps(ut, indent=2) + '\n')

def image(folder):
    return {str(path.relative_to(folder)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in folder.rglob('*') if path.is_file()}

previous, current = image(origin / 'apps'), image(source / 'apps')
generated = {'before_files': len(previous), 'after_files': len(current),
             'missing': sorted(previous.keys() - current.keys()),
             'added': sorted(current.keys() - previous.keys()),
             'changed': sorted(name for name in previous.keys() & current.keys()
                               if previous[name] != current[name])}
(artifacts / 'generated-image.json').write_text(json.dumps(generated, indent=2) + '\n')

for name, tree in (('own', source), ('main', repository)):
    entries = (tree / 'build/first-gap/files').read_text().splitlines()
    declared = {str(Path(path).relative_to(tree / 'apps')) for path in entries}
    actual = {str(path.relative_to(tree / 'apps')) for path in (tree / 'apps').rglob('*.cpp')
              if path.is_file()}
    if declared != actual or len(declared) != len(entries):
        raise RuntimeError(f'{name} body inventory does not reconcile')
    shutil.copy2(tree / 'build/first-gap/files', artifacts / f'body-files-{name}')
shutil.copy2(source / 'build/body-gap-first.log', artifacts / 'body-gap-own.log')

lane = repository / 'build/verify/lane/source'
same_code = {name: image(repository / name) == image(lane / name) for name in ('src', 'include')}
same_code['CMakeLists.txt'] = (repository / 'CMakeLists.txt').read_bytes() == (lane / 'CMakeLists.txt').read_bytes()
if not all(same_code.values()):
    raise RuntimeError('borrowed GCC lane is not the current production compiler source')
same_tools = {'script': (repository / 'scripts/first_gap.sh').read_bytes() ==
                       (source / 'scripts/first_gap.sh').read_bytes(),
              'gap_recipe': (repository / 'Makefile').read_text().split('gap: db')[1].split('# BARE')[0] ==
                            (source / 'Makefile').read_text().split('gap: db')[1].split('# BARE')[0],
              'controls': (repository / 'test/toolchain.py').read_text().split('class FirstGapGate')[1].split('class SeedTransferGate')[0] ==
                          (source / 'test/toolchain.py').read_text().split('class FirstGapGate')[1].split('class SeedTransferGate')[0]}
if not all(same_tools.values()):
    raise RuntimeError('tool integration differs from tested implementation')
integration = {'matching_gcc_build': str(lane / 'build/gcc'),
               'production_compiler_inputs_match': same_code, 'tested_tool_paths_match': same_tools,
               'stale_main_gcc_result_retained': True,
               'no_runtime_or_generator_integration': True, 'frozen_integration_run': False}
(artifacts / 'integration.json').write_text(json.dumps(integration, indent=2) + '\n')

exits = {'copy': 0, 'controls_old': 1, 'controls_current_final': 0,
         'controls_cases': 16, 'old_affected_cases': 14, 'old_failed_assertions': 17,
         'own_toolchain_clang': 0, 'own_toolchain_gcc': 0, 'own_toolchain_cases': 106,
         'main_toolchain_clang': 0, 'main_toolchain_gcc_stale': 1,
         'main_toolchain_gcc_matching_source': 0, 'main_toolchain_cases': 99,
         'body_gap_own_make': 2, 'body_gap_main_make': 2, 'body_sweep_script': 1,
         'body_clang_no_pch': 1, 'body_gcc_no_pch': 1,
         'own_bodies': {'total': 14465, 'passed_prefix': 1, 'first_failed': 1, 'unmeasured': 14463},
         'main_bodies': {'total': 14464, 'passed_prefix': 1, 'first_failed': 1, 'unmeasured': 14462},
         'first_failed': 'base/accountant_portal/codeunit/InviteExternalAccountant.cpp',
         'full_body_success': False, 'no_ut_execution': True,
         'shell_syntax': 0, 'diff_check': 0}
(root / 'proof-exits.json').write_text(json.dumps(exits, indent=2) + '\n')
print(json.dumps({'source_sha256': identity['source_sha256'], 'ut': ut,
                  'generated_image': generated, 'integration': integration}, indent=2))
