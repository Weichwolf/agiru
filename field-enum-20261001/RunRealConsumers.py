from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
old = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
report = Path('apps/base/system/environment/configuration/page/ReportResGovernSettings.cpp')
feature = Path('apps/system/system/environment/configuration/page/FeatureManagement.cpp')
rows = []
feature_rows = []
for compiler, cxx in [('clang', 'clang++-19'), ('gcc', 'g++-14')]:
    flags = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror']
    for label, image in [('predecessor', old), ('current', source)]:
        includes = ['-I' + str(image / 'include')]
        includes += ['-I' + str(image / 'apps' / app) for app in
                     ('base', 'system', 'foundation', 'shared', 'absent')]
        command = flags + ['-fsyntax-only', *includes, str(image / report)]
        result = subprocess.run(command, text=True, capture_output=True, timeout=90)
        valid = (result.returncode == 0 if label == 'current' else result.returncode != 0 and
                 'OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701' in result.stderr)
        rows.append({'compiler': compiler, 'image': label, 'command': command,
                     'exit': result.returncode, 'diagnostics': result.stderr, 'valid': valid})
        build = image / ('build' if compiler == 'clang' else 'build/gcc')
        executable = root / 'artifacts' / ('real-feature-' + compiler + '-' + label)
        command = flags + ['-ffunction-sections', '-fdata-sections', *includes,
                           '-I' + str(source / 'test/gate'), str(root / 'RealFeatureControl.cpp'),
                           str(image / feature), '-L' + str(build), '-Wl,-rpath,' + str(build),
                           '-Wl,--gc-sections', '-lagiru_rt', '-lagiru_net', '-lagiru_db',
                           '-o', str(executable)]
        compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
        executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
                    if compiled.returncode == 0 else None)
        feature_rows.append({'compiler': compiler, 'image': label, 'command': command,
                             'compile_exit': compiled.returncode, 'diagnostics': compiled.stderr,
                             'execution_exit': executed.returncode if executed else None,
                             'stdout': executed.stdout if executed else None,
                             'stderr': executed.stderr if executed else None,
                             'valid': compiled.returncode == 0 and executed is not None and
                             executed.returncode == 0 and '4 check(s), 0 red' in executed.stdout})
receipt = {'report': rows, 'feature': feature_rows, 'no_pch': True,
           'untouched_full_generated_consumer': True, 'sql_or_web_client_proof': False,
           'feature_old_image_is_not_a_negative_control': True,
           'success': all(row['valid'] for row in rows + feature_rows)}
(root / 'artifacts/real-consumers.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'report_checks': len(rows), 'feature_checks': len(feature_rows),
                  'success': receipt['success']}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
