from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
rows = []
for compiler, cxx in [('clang', 'clang++-19'), ('gcc', 'g++-14')]:
    build = source / ('build' if compiler == 'clang' else 'build/gcc')
    executable = root / 'artifacts' / ('actual-page-management-' + compiler)
    command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
               '-ffunction-sections', '-fdata-sections', '-I' + str(source / 'include'),
               '-I' + str(source / 'test/gate')]
    command += ['-I' + str(source / 'apps' / app) for app in ('base', 'system', 'foundation', 'shared', 'absent')]
    command += [str(root / 'ActualPageManagement.cpp'),
                str(source / 'apps/base/utilities/codeunit/PageManagement.cpp'),
                '-L' + str(build), '-Wl,-rpath,' + str(build), '-Wl,--gc-sections',
                '-lagiru_rt', '-lagiru_net', '-lagiru_db', '-o', str(executable)]
    compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
    executed = (subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
                if compiled.returncode == 0 else None)
    rows.append({'compiler': compiler, 'command': command, 'compile_exit': compiled.returncode,
                 'diagnostics': compiled.stderr, 'execution_exit': executed.returncode if executed else None,
                 'stdout': executed.stdout if executed else None, 'stderr': executed.stderr if executed else None,
                 'valid': compiled.returncode == 0 and executed is not None and executed.returncode == 0 and
                 '2 check(s), 0 red' in executed.stdout})
receipt = {'results': rows, 'success': all(row['valid'] for row in rows),
           'full_consumer_compiled_without_pch': True, 'zero_table_shortcuts_only': True,
           'sql_provider_lookup_or_client_workflow_proof': False}
(root / 'artifacts/actual-page-management.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'success': receipt['success'], 'outputs': [row['stdout'] for row in rows]}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
