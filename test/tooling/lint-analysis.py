#!/usr/bin/env python3
"""Select real translation units and preserve clang-tidy failures."""
import argparse
import concurrent.futures
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys


def compile_entries(root):
    entries = json.loads((root / 'compile_commands.json').read_text())
    database_root = (root / 'compile_commands.json').resolve().parent
    build = database_root if database_root != root else root / 'build'
    for receipt in sorted((build / 'fixture-commands').glob('*.json')):
        entries.extend(json.loads(receipt.read_text()))
    return entries


def select_units(root, full, unit=None):
    entries = compile_entries(root)
    units = {}
    for entry in entries:
        path = Path(entry['file']).resolve()
        relative = path.relative_to(root) if path.is_relative_to(root) else None
        if relative is not None and (relative.parts[0] == 'src' or (
                relative.parts[0] == 'test' and not path.is_relative_to(
                    root / 'test/transpiler/golden'))):
            units[path] = entry
    if not units:
        raise RuntimeError('compile_commands.json contains no hand-written translation units')
    if unit is not None:
        selected = (root / unit).resolve()
        if selected not in units or not selected.is_file():
            raise RuntimeError(f'unit is not a compiled handwritten source: {unit}')
        return [selected], len(units)
    expected = {p.resolve() for p in (root / 'src').rglob('*.cpp')}
    expected.update(p.resolve() for p in (root / 'test').rglob('*.cpp')
                    if not p.is_relative_to(root / 'test/transpiler/golden'))
    missing_commands = expected - units.keys()
    if missing_commands:
        raise RuntimeError('hand-written source has no compile command: ' + ', '.join(map(str, sorted(missing_commands))))
    stale = {p for p in units if not p.is_file()}
    if stale:
        raise RuntimeError('compile command refers to missing source: ' + ', '.join(map(str, sorted(stale))))
    if full:
        return sorted(units), len(units)
    changed = set()
    for command in (
        ['git', 'diff', '--name-only', '-z', 'HEAD', '--', 'src', 'include', 'test'],
        ['git', 'ls-files', '--others', '--exclude-standard', '-z', '--', 'src', 'include', 'test'],
    ):
        for name in subprocess.check_output(command, cwd=root).split(b'\0'):
            if name:
                path = root / os.fsdecode(name)
                if path.is_file() and path.suffix in ('.h', '.cpp') and not path.is_relative_to(root / 'test/transpiler/golden'):
                    changed.add(path.resolve())
    selected = changed.intersection(units)
    missing = {path for path in changed if path.suffix == '.cpp'} - units.keys()
    if missing:
        raise RuntimeError('changed source has no compile command: ' + ', '.join(map(str, sorted(missing))))
    headers = {path for path in changed if path.suffix == '.h'}
    if headers:
        # The compiler's dependency graph includes transitive headers and avoids guessing includes.
        deps = subprocess.check_output(['ninja', '-C', str(root / 'build'), '-t', 'deps'], text=True)
        objects = {}
        for path, entry in units.items():
            args = entry.get('arguments') or shlex.split(entry['command'])
            if '-o' in args:
                obj = Path(entry['directory']) / args[args.index('-o') + 1]
                objects[obj.resolve()] = path
        matched = set()
        current = None
        for line in deps.splitlines():
            if line and not line.startswith(' '):
                obj = root / 'build' / line.split(': #deps', 1)[0]
                current = objects.get(obj.resolve())
            elif current is not None and line.strip():
                dep = (root / 'build' / line.strip()).resolve()
                if dep in headers:
                    selected.add(current)
                    matched.add(dep)
        if matched != headers:
            raise RuntimeError('changed header has no compiled consumer: ' + ', '.join(map(str, sorted(headers - matched))))
    return sorted(selected), len(units)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument('--tidy', required=True)
    parser.add_argument('--full', action='store_true')
    parser.add_argument('--unit', type=Path,
                        default=Path(os.environ['AGIRU_LINT_UNIT']) if os.environ.get('AGIRU_LINT_UNIT') else None,
                        help='one compiled handwritten .cpp for the local edit loop')
    parser.add_argument('--require-unit', action='store_true')
    parser.add_argument('--jobs', type=int, default=os.cpu_count() or 1)
    parser.add_argument('--nodes', type=int, default=50000)
    parser.add_argument('--padding', type=int, default=64)
    args = parser.parse_args()
    root = args.root.resolve()
    report = root / 'build/lint'
    report.mkdir(parents=True, exist_ok=True)
    try:
        if args.require_unit and args.unit is None:
            raise RuntimeError('set UNIT to a compiled handwritten .cpp')
        if args.unit is not None:
            if args.full:
                raise RuntimeError('--unit and --full cannot be combined')
            units, total = select_units(root, True, args.unit)
        else:
            units, total = select_units(root, args.full)
        if args.jobs < 1:
            raise RuntimeError('jobs must be positive')
        extra = ['--extra-arg=-Xclang', '--extra-arg=-analyzer-config', '--extra-arg=-Xclang']
        budget = extra + [f'--extra-arg=max-nodes={args.nodes}']
        budget += extra + [f'--extra-arg=optin.performance.Padding:AllowedPad={args.padding}']
        (report / 'compile_commands.json').write_text(json.dumps(compile_entries(root)) + '\n')

        def check(path):
            result = subprocess.run([args.tidy, '-p', str(report), '--quiet', *budget, str(path)],
                                    cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            return path, result.returncode, result.stdout

        failed = 0
        stem = 'targeted' if args.unit is not None else 'tidy'
        with (report / f'{stem}.log').open('w') as log, concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
            for path, status, output in pool.map(check, units):
                log.write(f'== {path.relative_to(root)} (exit {status}) ==\n{output}\n')
                failed += status != 0
        units_name = 'targeted-units.json' if args.unit is not None else 'units.json'
        (report / units_name).write_text(json.dumps({'available': total, 'checked': len(units),
                                                     'failed': failed, 'files': [str(p.relative_to(root)) for p in units]}, indent=2) + '\n')
        print(f'lint: analysed {len(units)} of {total} hand-written translation units; {failed} failed')
        return 1 if failed else 0
    except (OSError, ValueError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f'lint: analysis aborted: {error}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    sys.exit(main())
