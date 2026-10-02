import json
from pathlib import Path
import shutil
import sys

repository = Path(__file__).resolve().parents[2]
build = repository / 'build'
current = Path(__file__).resolve().parent
generated_files = {'CMakeCache.txt', 'build.ninja', 'cmake_install.cmake',
                   'compile_commands.json', '.ninja_log', '.ninja_deps',
                   'CTestTestfile.cmake', 'install_manifest.txt'}
generated_dirs = {'CMakeFiles', 'tree-cache'}
roots = sorted({cache.parent for cache in build.rglob('CMakeCache.txt')
                if not cache.is_symlink() and not cache.is_relative_to(current)})
targets = []
for root in roots:
    assert root.resolve().is_relative_to(build.resolve())
    for path in sorted(root.iterdir()):
        if path.name in generated_files or path.name in generated_dirs:
            targets.append(path)
        elif path.is_file() and (path.name in ('agiru', 'agirutc') or
                                 path.name.startswith('gate_') or
                                 (path.name.startswith('libagiru_') and '.so' in path.name)):
            with path.open('rb') as stream:
                assert stream.read(4) == b'\x7fELF', f'unexpected non-binary: {path}'
            targets.append(path)
targets = sorted(set(targets))
for path in targets:
    assert path != build and path != repository and not path.is_relative_to(current)
    assert path.parent.resolve().is_relative_to(build.resolve())
    assert not path.is_symlink(), f'linked output requires separate inspection: {path}'
plan = {'scope': 'Obsolete CMake compiler products/caches only; sources, fixtures, receipts and current LLVM build retained.',
        'targets': [str(path) for path in targets], 'configured_roots': [str(root) for root in roots],
        'removed': False}
receipt = current / 'obsolete-compiler-cleanup.json'
if '--apply' in sys.argv:
    for path in targets:
        if path.is_dir():
            shutil.rmtree(path)
        else:
            path.unlink()
    plan['removed'] = True
    plan['remaining_targets'] = [str(path) for path in targets if path.exists()]
    assert not plan['remaining_targets']
    for root in sorted(roots, key=lambda path: len(path.parts), reverse=True):
        if root != build and root.is_dir() and not any(root.iterdir()):
            root.rmdir()
receipt.write_text(json.dumps(plan, indent=2) + '\n')
print(json.dumps({'targets': len(targets), 'configured_roots': len(roots), 'removed': plan['removed'],
                  'receipt': str(receipt)}))
