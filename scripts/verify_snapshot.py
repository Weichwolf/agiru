#!/usr/bin/env python3
"""Freeze the current worktree, then verify that copy without blocking source edits."""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time


ROOT = Path(__file__).resolve().parents[1]
EXCLUDED = {'.git', 'build', 'build-asan', 'work', 'compile_commands.json'}
CACHE_DIRS = {'__pycache__', '.pytest_cache'}


def files(root):
    for base, directories, names in os.walk(root, followlinks=False):
        if Path(base) == root:
            directories[:] = sorted(name for name in directories
                                    if name not in EXCLUDED | CACHE_DIRS)
            names = [name for name in names if name not in EXCLUDED]
        else:
            directories[:] = sorted(name for name in directories if name not in CACHE_DIRS)
        for name in sorted(names):
            if name.endswith('.pyc'):
                continue
            path = Path(base) / name
            yield path.relative_to(root)


def remove_caches(root):
    for base, directories, names in os.walk(root, followlinks=False):
        if Path(base) == root:
            directories[:] = [name for name in directories if name not in EXCLUDED]
        for name in list(directories):
            if name in CACHE_DIRS:
                shutil.rmtree(Path(base) / name)
                directories.remove(name)
        for name in names:
            if name.endswith('.pyc'):
                (Path(base) / name).unlink()


def digest(root):
    image = hashlib.sha256()
    for relative in files(root):
        path = root / relative
        image.update(str(relative).encode())
        image.update(b'\0')
        if path.is_symlink():
            image.update(b'L')
            image.update(os.readlink(path).encode())
        else:
            image.update(b'F')
            with path.open('rb') as stream:
                for block in iter(lambda: stream.read(1024 * 1024), b''):
                    image.update(block)
    return image.hexdigest()


def copy_source(source, destination):
    destination.mkdir(parents=True, exist_ok=True)
    for child in source.iterdir():
        if child.name in EXCLUDED:
            continue
        target = destination / child.name
        if child.is_dir() and not child.is_symlink():
            subprocess.run(['cp', '-a', '--reflink=auto', str(child) + '/.', str(target)],
                           check=True)
        else:
            subprocess.run(['cp', '-a', '--reflink=auto', str(child), str(target)], check=True)
    remove_caches(destination)


def has_links(root):
    return root.is_symlink() or any(path.is_symlink() for path in root.rglob('*'))


def freeze_input(source, destination, name):
    if not source.is_dir():
        raise RuntimeError(f'{name} input is not a directory: {source}')
    if has_links(source):
        raise RuntimeError(f'{name} input contains an unfrozen symlink')
    before = digest(source)
    copy_source(source, destination)
    frozen = digest(destination)
    if has_links(destination):
        raise RuntimeError(f'{name} frozen input contains an unfrozen symlink')
    if frozen != before or digest(source) != before:
        raise RuntimeError(f'{name} source changed during snapshot; verification was not started')
    return frozen


def same_file(left, right):
    if right.is_symlink() or not right.is_file() or left.stat().st_size != right.stat().st_size:
        return False
    with left.open('rb') as a, right.open('rb') as b:
        while True:
            first = a.read(1024 * 1024)
            if first != b.read(1024 * 1024):
                return False
            if not first:
                return True


def sync_source(source, destination):
    remove_caches(destination)
    wanted = set(files(source))
    for relative in wanted:
        original = source / relative
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        if original.is_symlink():
            link = os.readlink(original)
            if target.is_symlink() and os.readlink(target) == link:
                continue
            target.unlink(missing_ok=True)
            target.symlink_to(link)
        elif not same_file(original, target):
            if target.is_symlink():
                target.unlink()
            shutil.copy2(original, target)
            os.utime(target, None)
    for relative in set(files(destination)) - wanted:
        (destination / relative).unlink()


def prepare_lane(parent, archive, head, expected):
    lane = parent / 'lane'
    lane.mkdir(exist_ok=True)
    latest = lane / 'latest'
    if latest.exists():
        previous = Path(latest.read_text().strip()) / 'result.json'
        if json.loads(previous.read_text())['status'] not in ('passed', 'failed'):
            raise RuntimeError('the integration lane is still running')
    source = lane / 'source'
    if not source.exists():
        subprocess.run(['git', '-C', str(ROOT), 'worktree', 'add', '--detach', str(source), head],
                       check=True, stdout=subprocess.DEVNULL)
    else:
        current = subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'],
                                          text=True).strip()
        if current != head:
            subprocess.run(['git', '-C', str(source), 'reset', '--mixed', '-q', head],
                           check=True, stdout=subprocess.DEVNULL)
    sync_source(archive, source)
    if digest(source) != expected:
        raise RuntimeError('the reusable integration lane differs from its frozen source')
    return source


def write_json(path, value):
    pending = path.with_suffix('.pending')
    pending.write_text(json.dumps(value, indent=2, sort_keys=True) + '\n')
    pending.replace(path)


def start(arguments):
    stamp = time.strftime('%Y%m%dT%H%M%SZ', time.gmtime())
    parent = ROOT / 'build/verify'
    parent.mkdir(parents=True, exist_ok=True)
    run = parent / f'{stamp}-{os.getpid()}'
    source = run / 'source'
    run.mkdir()
    head = subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'],
                                   text=True).strip()
    subprocess.run(['git', '-C', str(ROOT), 'worktree', 'add', '--detach', str(source), head],
                   check=True, stdout=subprocess.DEVNULL)
    try:
        copy_source(ROOT, source)
        tracked = subprocess.check_output(['git', '-C', str(source), 'ls-files', '-z']).split(b'\0')
        for item in tracked:
            if item and not (ROOT / os.fsdecode(item)).exists():
                (source / os.fsdecode(item)).unlink(missing_ok=True)
        before = digest(ROOT)
        after = digest(source)
        if before != after:
            raise RuntimeError('source changed during snapshot; verification was not started')
        metadata = {'head': head, 'source_sha256': after, 'source': str(source),
                    'log': str(run / 'verify.log'),
                    'targets': arguments.targets, 'jobs': arguments.jobs,
                    'created_utc': stamp, 'status': 'queued'}
        if 'ut' in arguments.targets or 'transpile' in arguments.targets:
            bc = Path(os.environ.get('AGIRU_BC_SOURCE', Path.home() / 'Git/BCApps/src'))
            bc_copy = run / 'bc_source'
            metadata['bc_source_sha256'] = freeze_input(bc, bc_copy, 'BCApps')
            revision = subprocess.run(['git', '-C', str(bc), 'rev-parse',
                                       '--show-toplevel', 'HEAD'],
                                      capture_output=True, text=True, check=False)
            lines = revision.stdout.splitlines()
            metadata['bc_source_revision'] = (
                lines[1] if revision.returncode == 0 and len(lines) == 2
                and Path(lines[0]).resolve() != ROOT.resolve() else None)
        if configured := os.environ.get('AGIRU_SYSTEM_SYMBOLS'):
            symbols = Path(configured).resolve()
            if not all((symbols / name).is_file() for name in
                       ('NavxManifest.xml', 'SymbolReference.json')):
                raise RuntimeError('System symbols input lacks its manifest or symbol reference')
            if not (symbols / 'src').is_dir():
                raise RuntimeError('System symbols input lacks its AL source directory')
            metadata['system_symbols_sha256'] = freeze_input(
                symbols, run / 'system_symbols', 'System symbols')
    except Exception:
        subprocess.run(['git', '-C', str(ROOT), 'worktree', 'remove', '--force', str(source)],
                       check=False)
        shutil.rmtree(run)
        raise
    def launch():
        if arguments.reuse:
            metadata['build_source'] = str(prepare_lane(parent, source, head, after))
        write_json(run / 'result.json', metadata)
        command = [sys.executable, str(source / 'scripts/verify_snapshot.py'), 'run', str(run)]
        if arguments.detach:
            with (run / 'verify.log').open('w') as output:
                subprocess.Popen(command, cwd=source, stdout=output,
                                 stderr=subprocess.STDOUT, start_new_session=True)
        (parent / 'latest').write_text(str(run) + '\n')
        if arguments.reuse:
            (parent / 'lane/latest').write_text(str(run) + '\n')

    try:
        if arguments.reuse:
            lock_path = parent / 'lane/prepare.lock'
            lock_path.parent.mkdir(exist_ok=True)
            with lock_path.open('w') as lock:
                fcntl.flock(lock, fcntl.LOCK_EX)
                launch()
        else:
            launch()
    except Exception:
        subprocess.run(['git', '-C', str(ROOT), 'worktree', 'remove', '--force', str(source)],
                       check=False)
        shutil.rmtree(run)
        raise
    if arguments.detach:
        print(run)
        return 0
    return run_snapshot(run)


def run_snapshot(run):
    metadata_path = run / 'result.json'
    metadata = json.loads(metadata_path.read_text())
    metadata['pid'] = os.getpid()
    metadata['status'] = 'running'
    metadata['artifacts'] = str(run / 'artifacts')
    write_json(metadata_path, metadata)
    environment = dict(os.environ)
    environment.pop('AGIRU_BC_REVISION', None)
    environment.pop('AGIRU_SYSTEM_SYMBOLS', None)
    build_source = Path(metadata.get('build_source', run / 'source'))
    if (run / 'bc_source').exists():
        environment['AGIRU_BC_SOURCE'] = str(run / 'bc_source')
        if metadata.get('bc_source_revision'):
            environment['AGIRU_BC_REVISION'] = metadata['bc_source_revision']
    start_time = time.monotonic()
    outcomes = {}
    symbols = run / 'system_symbols'
    expected_symbols = metadata.get('system_symbols_sha256')
    if expected_symbols:
        if not symbols.is_dir() or has_links(symbols) or digest(symbols) != expected_symbols:
            outcomes['system_symbols_immutable'] = 1
        else:
            environment['AGIRU_SYSTEM_SYMBOLS'] = str(symbols)
    with (run / 'verify.log').open('a') as output:
        for target in metadata['targets']:
            output.write(f'VERIFY TARGET {target}\n')
            output.flush()
            if outcomes.get('system_symbols_immutable'):
                output.write('VERIFY REFUSED: frozen System symbols identity differs\n')
                outcomes[target] = 2
                continue
            command = ['make', '-C', str(build_source), f'JOBS={metadata["jobs"]}', target]
            if target == 'ut':
                command.append(f'UT_LOG={run / "artifacts/ut.log"}')
            try:
                outcomes[target] = subprocess.run(command, env=environment, stdout=output,
                                                   stderr=subprocess.STDOUT, check=False).returncode
            except OSError as error:
                output.write(f'VERIFY TARGET {target} could not start: {error}\n')
                outcomes[target] = 2
            output.flush()
    metadata['post_source_sha256'] = digest(build_source)
    if metadata['post_source_sha256'] != metadata['source_sha256']:
        outcomes['source_immutable'] = 1
    if expected_symbols:
        metadata['post_system_symbols_sha256'] = digest(symbols)
        if metadata['post_system_symbols_sha256'] != expected_symbols or has_links(symbols):
            outcomes['system_symbols_immutable'] = 1
    status = 0 if all(code == 0 for code in outcomes.values()) else 1
    metadata['target_exits'] = outcomes
    metadata['status'] = 'passed' if status == 0 else 'failed'
    metadata['exit_code'] = status
    metadata['elapsed_seconds'] = int(time.monotonic() - start_time)
    metadata['finished_utc'] = time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())
    write_json(metadata_path, metadata)
    print(f'{run}: {metadata["status"]} ({metadata["elapsed_seconds"]} s)')
    return status


def clean_snapshots():
    parent = ROOT / 'build/verify'
    if not parent.exists():
        return 0
    runs = sorted(path for path in parent.iterdir() if path.is_dir() and path.name != 'lane')
    for run in runs:
        result = run / 'result.json'
        if not result.exists():
            raise RuntimeError(f'{run} has no result; inspect it before cleaning')
        state = json.loads(result.read_text())['status']
        if state not in ('passed', 'failed'):
            raise RuntimeError(f'{run} is {state}; wait for it before cleaning')
    for run in runs:
        source = run / 'source'
        subprocess.run(['git', '-C', str(ROOT), 'worktree', 'remove', '--force', str(source)],
                       check=True)
        shutil.rmtree(run)
    lane = parent / 'lane'
    if (lane / 'source').exists():
        subprocess.run(['git', '-C', str(ROOT), 'worktree', 'remove', '--force',
                        str(lane / 'source')], check=True)
    if lane.exists():
        shutil.rmtree(lane)
    (parent / 'latest').unlink(missing_ok=True)
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    begin = sub.add_parser('start')
    begin.add_argument('--jobs', type=int, default=6)
    begin.add_argument('--detach', action='store_true')
    begin.add_argument('--reuse', action='store_true', help='reuse the integration lane build tree')
    begin.add_argument('targets', nargs='+')
    run = sub.add_parser('run')
    run.add_argument('snapshot', type=Path)
    status = sub.add_parser('status')
    status.add_argument('snapshot', type=Path, nargs='?')
    sub.add_parser('clean')
    arguments = parser.parse_args()
    if arguments.command == 'start':
        if arguments.jobs < 1:
            parser.error('jobs must be positive')
        return start(arguments)
    if arguments.command == 'run':
        return run_snapshot(arguments.snapshot)
    if arguments.command == 'clean':
        return clean_snapshots()
    snapshot = arguments.snapshot or Path((ROOT / 'build/verify/latest').read_text().strip())
    print((snapshot / 'result.json').read_text(), end='')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, subprocess.CalledProcessError, RuntimeError) as error:
        print(f'verification snapshot: {error}', file=sys.stderr)
        sys.exit(2)
