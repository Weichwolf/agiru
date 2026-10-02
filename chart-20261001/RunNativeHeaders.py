from pathlib import Path
import hashlib
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
origin = Path(json.loads((root / 'source-identity.json').read_text())['origin'])
original = Path('/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH/src/Application Database Tables/ODataEdmType.Table.al')
rows = []
for compiler, cxx in [('clang', 'clang++-19'), ('gcc', 'g++-14')]:
    for label, image in [('predecessor', origin), ('current', source)]:
        build = image / ('build' if compiler == 'clang' else 'build/gcc')
        output = root / 'artifacts' / ('native-headers-' + compiler + '-' + label)
        driver = root / 'artifacts' / ('emit-native-headers-' + compiler + '-' + label)
        command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                   '-I' + str(image / 'src/al'), '-I' + str(image / 'src/gen'),
                   '-I' + str(image / 'include'), '-I' + str(source / 'test/gate'),
                   str(root / 'NativeHeaders.cpp'), '-L' + str(build),
                   '-Wl,-rpath,' + str(build), '-lagiru_gen', '-lagiru_al', '-o', str(driver)]
        compiled = subprocess.run(command, text=True, capture_output=True, timeout=120)
        if compiled.returncode != 0:
            raise SystemExit(compiled.stderr)
        emitted = subprocess.run([str(driver), str(original), str(output)], text=True,
                                 capture_output=True, timeout=30)
        syntax_command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                          '-fsyntax-only', '-I' + str(image / 'include'), '-I' + str(output),
                          str(root / 'OwnedNativeHeaders.cpp')]
        syntax = subprocess.run(syntax_command, text=True, capture_output=True, timeout=120)
        mutant = output / 'without-owned-include'
        mutant.mkdir(exist_ok=True)
        removed = 0
        for header in ('NativeGlobal.h', 'NativeParameter.h', 'NativePage.h'):
            text = (output / header).read_text()
            line = '#include "platform/ODataEdmType.h"\n'
            if text.count(line) != 1:
                raise SystemExit('owned native include count changed: ' + header)
            (mutant / header).write_text(text.replace(line, ''))
            removed += 1
        negative_command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                            '-fsyntax-only', '-I' + str(image / 'include'), '-I' + str(mutant),
                            str(root / 'OwnedNativeHeaders.cpp')]
        negative = subprocess.run(negative_command, text=True, capture_output=True, timeout=120)
        valid = (emitted.returncode == 0 and '3 check(s), 0 red' in emitted.stdout and
                 syntax.returncode == 0 and removed == 3 and negative.returncode != 0 and
                 'ODataEdmType' in negative.stderr)
        rows.append({'compiler': compiler, 'image': label, 'emit_command': command,
                     'emit_exit': emitted.returncode, 'emit_stdout': emitted.stdout,
                     'emit_stderr': emitted.stderr, 'syntax_command': syntax_command,
                     'syntax_exit': syntax.returncode, 'syntax_diagnostics': syntax.stderr,
                     'negative_command': negative_command, 'removed_owned_includes': removed,
                     'negative_exit': negative.returncode, 'negative_diagnostics': negative.stderr,
                     'valid': valid})
        (root / 'artifacts/native-headers.json').write_text(json.dumps({
            'results': rows, 'original_source': str(original),
            'original_sha256': hashlib.sha256(original.read_bytes()).hexdigest(),
            'pre_existing_native_binding_in_both_images': True,
            'no_chart_binding_or_header_dependency': True,
            'predecessor_legacy_Door_fixed_type_list_masks_original_header_gap': True,
            'predecessor_expected_red': 0,
            'no_pch': True,
            'success': len(rows) == 4 and all(row['valid'] for row in rows)
        }, indent=2) + '\n')
        if not valid:
            raise SystemExit(json.dumps(rows[-1]))
print(json.dumps({'success': True, 'checks_per_image': 3, 'predecessor_red': 0,
                  'erased_owned_includes_per_control': 3}), flush=True)
