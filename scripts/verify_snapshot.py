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


def files(root):
    for base, directories, names in os.walk(root, followlinks=False):
        if Path(base) == root:
            directories[:] = sorted(name for name in directories if name not in EXCLUDED)
            names = [name for name in names if name not in EXCLUDED]
        else:
            directories.sort()
        for name in sorted(names):
            path = Path(base) / name
            yield path.relative_to(root)


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
        subprocess.run(['git', '-C', str(source), 'checkout', '--detach', '-f', head],
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
                (source / os.fsdecode(item)).unlink()
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
            copy_source(bc, bc_copy)
            if digest(bc) != digest(bc_copy):
                raise RuntimeError('BCApps source changed during snapshot; verification was not started')
            metadata['bc_source_sha256'] = digest(bc_copy)
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
                process = subprocess.Popen(command, cwd=source, stdout=output,
                                           stderr=subprocess.STDOUT, start_new_session=True)
            metadata['pid'] = process.pid
            write_json(run / 'result.json', metadata)
        (parent / 'latest').write_text(str(run) + '\n')
        if arguments.reuse:
            (parent / 'lane/latest').write_text(str(run) + '\n')

    if arguments.reuse:
        lock_path = parent / 'lane/prepare.lock'
        lock_path.parent.mkdir(exist_ok=True)
        with lock_path.open('w') as lock:
            fcntl.flock(lock, fcntl.LOCK_EX)
            launch()
    else:
        launch()
    if arguments.detach:
        print(run)
        return 0
    return run_snapshot(run)


def run_snapshot(run):
    metadata_path = run / 'result.json'
    metadata = json.loads(metadata_path.read_text())
    metadata['status'] = 'running'
    write_json(metadata_path, metadata)
    environment = dict(os.environ)
    build_source = Path(metadata.get('build_source', run / 'source'))
    if (run / 'bc_source').exists():
        environment['AGIRU_BC_SOURCE'] = str(run / 'bc_source')
    start_time = time.monotonic()
    outcomes = {}
    with (run / 'verify.log').open('a') as output:
        for target in metadata['targets']:
            output.write(f'VERIFY TARGET {target}\n')
            output.flush()
            command = ['make', '-C', str(build_source), f'JOBS={metadata["jobs"]}', target]
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
