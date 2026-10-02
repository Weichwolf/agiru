from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
unit = source / 'apps/base/system/environment/configuration/page/ReportResGovernSettings.cpp'
rows = []
for compiler, cxx in [('clang', 'clang++-19'), ('gcc', 'g++-14')]:
    for label, image in [('predecessor', origin), ('current', source)]:
        command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-fsyntax-only',
                   '-I' + str(image / 'include')]
        command += ['-I' + str(source / 'apps' / app) for app in
                    ('base', 'system', 'foundation', 'shared', 'absent')]
        command.append(str(unit))
        result = subprocess.run(command, text=True, capture_output=True, timeout=90)
        valid = result.returncode != 0 and 'OptionTableDataTableBlankReportBlank4CodeunitXMLportMe_135775701' in result.stderr
        rows.append({'compiler': compiler, 'image': label, 'command': command,
                     'exit': result.returncode, 'diagnostics': result.stderr, 'valid': valid})
receipt = {'results': rows, 'success': all(row['valid'] for row in rows),
           'no_pch': True, 'diagnostic_only': True, 'execution_proof': False,
           'business_source_changed': False,
           'observed_scope': 'TableContext::Enumeration reconstructs a hashed type; CodeunitContext uses Objects.fieldEnums',
           'proposed_owner': 'one declaration-owned field-enum lookup shared by both contexts',
           'cause_fixed': False}
(root / 'artifacts/option-diagnostic.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'checks': len(rows), 'success': receipt['success'], 'fixed': False}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
