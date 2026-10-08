#!/usr/bin/env python3
"""Freeze the current worktree, then verify that copy without blocking source edits."""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import shlex
import subprocess
import sys
import time


ROOT = Path(__file__).resolve().parents[1]
EXCLUDED = {'.git', 'build', 'build-asan', 'work', 'compile_commands.json'}
CACHE_DIRS = {'__pycache__', '.pytest_cache'}
CONFIGURATION_KEYS = ('AGIRU_TEST_DSN', 'AGIRU_MASTER_DSN', 'AGIRU_AL_SOURCE',
                      'AGIRU_BC_SOURCE', 'AGIRU_BUILD_APPS', 'AGIRU_BUILD_SLICE',
                      'CMAKE_BUILD_TYPE')


def build_configuration(root):
    if not (root / 'CMakeLists.txt').is_file():
        return None
    selected = os.environ.get('B')
    build = root / selected if selected else (root / 'compile_commands.json').resolve().parent
    cache = build / 'CMakeCache.txt'
    settings = {}
    if cache.is_file():
        for line in cache.read_text().splitlines():
            key, separator, value = line.partition('=')
            name = key.split(':', 1)[0]
            if separator and name in CONFIGURATION_KEYS:
                settings[name] = value
    for name in CONFIGURATION_KEYS[:4]:
        if name in os.environ:
            settings[name] = os.environ[name]
    if any(any(unit in value for unit in ('\0', '\r', '\n')) for value in settings.values()):
        raise RuntimeError('CMake configuration contains a multiline or NUL value')
    if not all(settings.get(name) for name in CONFIGURATION_KEYS[:4]):
        raise RuntimeError('frozen verification requires a configured selected build or explicit '
                           'AGIRU_TEST_DSN, AGIRU_MASTER_DSN, AGIRU_AL_SOURCE and AGIRU_BC_SOURCE')
    for name in ('AGIRU_AL_SOURCE', 'AGIRU_BC_SOURCE'):
        path = (root / settings[name]).resolve()
        if not path.is_dir():
            raise RuntimeError(f'{name} configuration does not name an existing directory')
        settings[name] = str(path)
    return settings


def freeze_configuration(settings, run):
    path = run / 'cmake-configuration.json'
    descriptor = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    with os.fdopen(descriptor, 'w') as output:
        json.dump(settings, output, sort_keys=True)
        output.write('\n')
    return input_digest(path)


def cmake_arguments(settings):
    return shlex.join([f'-D{name}={value}' for name, value in settings.items()]).replace('$', '$$')


def configure_snapshot(run, build_source, metadata, environment, output):
    expected = metadata.get('cmake_configuration_sha256')
    if not expected:
        return None
    path = run / 'cmake-configuration.json'
    if (not path.is_file() or path.is_symlink() or path.stat().st_mode & 0o077 or
            input_digest(path) != expected):
        raise RuntimeError('frozen CMake configuration differs')
    settings = json.loads(path.read_text())
    if (not isinstance(settings, dict) or
            not all(settings.get(name) for name in CONFIGURATION_KEYS[:4]) or
            any(name not in CONFIGURATION_KEYS or not isinstance(value, str) or
                any(unit in value for unit in ('\0', '\r', '\n'))
                for name, value in settings.items())):
        raise RuntimeError('invalid frozen CMake configuration')
    if (run / 'bc_source').exists():
        original = Path(settings['AGIRU_BC_SOURCE'])
        try:
            relative = Path(settings['AGIRU_AL_SOURCE']).relative_to(original)
        except ValueError as error:
            raise RuntimeError('AL gate root is outside the frozen BC source tree') from error
        settings['AGIRU_BC_SOURCE'] = str(run / 'bc_source')
        settings['AGIRU_AL_SOURCE'] = str(run / 'bc_source' / relative)
    for name in CONFIGURATION_KEYS[:4]:
        environment[name] = settings[name]
    arguments = cmake_arguments(settings)
    output.write('VERIFY CONFIGURE: declared database/source/build settings\n')
    output.flush()
    return subprocess.run(['make', '-C', str(build_source), 'configure',
                           f'CMAKE_ARGS={arguments}'], env=environment, stdout=output,
                          stderr=subprocess.STDOUT, check=False).returncode


def verification_root(root=None):
    identity = hashlib.sha256(str((root or ROOT).resolve()).encode()).hexdigest()[:16]
    return Path('/tmp/agiru-verify') / identity


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


def input_digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else digest(path)


def freeze_input(source, destination, name):
    if not source.is_dir() and not source.is_file():
        raise RuntimeError(f'{name} input is not a file or directory: {source}')
    if has_links(source):
        raise RuntimeError(f'{name} input contains an unfrozen symlink')
    before = input_digest(source)
    if source.is_dir():
        copy_source(source, destination)
    else:
        shutil.copy2(source, destination)
    frozen = input_digest(destination)
    if has_links(destination):
        raise RuntimeError(f'{name} frozen input contains an unfrozen symlink')
    if frozen != before or input_digest(source) != before:
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


def receipt_state(path):
    result = json.loads(path.read_text())
    pid = result.get('pid')
    if result['status'] != 'running' or not isinstance(pid, int) or pid <= 0:
        return result['status']
    try:
        os.kill(pid, 0)
    except ProcessLookupError:
        result.update(status='failed', exit_code=2,
                      interruption='runner PID no longer exists; verification is incomplete',
                      finished_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()))
        result.setdefault('target_exits', {})['interrupted'] = 2
        write_json(path, result)
    except PermissionError:
        pass
    return result['status']


def prepare_lane(parent, archive, head, expected):
    lane = parent / 'lane'
    lane.mkdir(exist_ok=True)
    latest = lane / 'latest'
    if latest.exists():
        previous = Path(latest.read_text().strip()) / 'result.json'
        if receipt_state(previous) not in ('passed', 'failed'):
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
    parent = verification_root()
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
        configuration = build_configuration(ROOT)
        if configuration is not None:
            metadata['cmake_configuration_sha256'] = freeze_configuration(configuration, run)
            metadata['cmake_configuration_keys'] = sorted(configuration)
        if 'ut' in arguments.targets or 'transpile' in arguments.targets:
            bc = Path(os.environ.get('AGIRU_BC_SOURCE', Path.home() / 'Git/BCApps/src'))
            notice = Path(os.environ.get('AGIRU_LAYOUT_SOURCE_NOTICE', bc.parent / 'LICENSE'))
            if 'test' in arguments.targets and not notice.is_file():
                raise RuntimeError('the test target requires the original BC source notice; '
                                   'set AGIRU_LAYOUT_SOURCE_NOTICE for a relocated source tree: '
                                   + str(notice))
            bc_copy = run / 'bc_source'
            metadata['bc_source_sha256'] = freeze_input(bc, bc_copy, 'BCApps')
            revision = subprocess.run(['git', '-C', str(bc), 'rev-parse',
                                       '--show-toplevel', 'HEAD'],
                                      capture_output=True, text=True, check=False)
            lines = revision.stdout.splitlines()
            metadata['bc_source_revision'] = (
                lines[1] if revision.returncode == 0 and len(lines) == 2
                and Path(lines[0]).resolve() != ROOT.resolve() else None)
            if notice.exists() or 'AGIRU_LAYOUT_SOURCE_NOTICE' in os.environ:
                metadata['layout_source_notice_sha256'] = freeze_input(
                    notice, run / 'layout_source_notice', 'BC source notice')
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
    for name in ('MAKEFLAGS', 'MFLAGS', 'MAKEOVERRIDES', 'B', 'CMAKE_ARGS'):
        environment.pop(name, None)
    environment.pop('AGIRU_BC_REVISION', None)
    environment.pop('AGIRU_SYSTEM_SYMBOLS', None)
    environment.pop('AGIRU_LAYOUT_SOURCE_NOTICE', None)
    build_source = Path(metadata.get('build_source', run / 'source'))
    if (run / 'bc_source').exists():
        environment['AGIRU_BC_SOURCE'] = str(run / 'bc_source')
        if metadata.get('bc_source_revision'):
            environment['AGIRU_BC_REVISION'] = metadata['bc_source_revision']
    start_time = time.monotonic()
    outcomes = {}
    notice = run / 'layout_source_notice'
    expected_notice = metadata.get('layout_source_notice_sha256')
    if expected_notice:
        if not notice.is_file() or has_links(notice) or input_digest(notice) != expected_notice:
            outcomes['layout_source_notice_immutable'] = 1
        else:
            environment['AGIRU_LAYOUT_SOURCE_NOTICE'] = str(notice)
    symbols = run / 'system_symbols'
    expected_symbols = metadata.get('system_symbols_sha256')
    if expected_symbols:
        if not symbols.is_dir() or has_links(symbols) or digest(symbols) != expected_symbols:
            outcomes['system_symbols_immutable'] = 1
        else:
            environment['AGIRU_SYSTEM_SYMBOLS'] = str(symbols)
    with (run / 'verify.log').open('a') as output:
        try:
            configured = configure_snapshot(run, build_source, metadata, environment, output)
            if configured is not None:
                outcomes['configure'] = configured
        except (OSError, ValueError, RuntimeError) as error:
            output.write(f'VERIFY REFUSED: {error}\n')
            outcomes['configure'] = 2
        for target in metadata['targets']:
            output.write(f'VERIFY TARGET {target}\n')
            output.flush()
            if outcomes.get('system_symbols_immutable') or outcomes.get('layout_source_notice_immutable') or outcomes.get('configure'):
                output.write('VERIFY REFUSED: frozen dependency identity differs\n')
                outcomes[target] = 2
                continue
            command = ['make', '-C', str(build_source), f'JOBS={metadata["jobs"]}', target]
            if target == 'ut':
                command.append(f'UT_LOG={run / "artifacts/ut.log"}')
            elif target == 'census':
                command.append(f'B={run / "artifacts/census"}')
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
    expected_configuration = metadata.get('cmake_configuration_sha256')
    if expected_configuration:
        configuration = run / 'cmake-configuration.json'
        metadata['post_cmake_configuration_sha256'] = (
            input_digest(configuration) if configuration.is_file() else None)
        if (configuration.is_symlink() or
                metadata['post_cmake_configuration_sha256'] != expected_configuration):
            outcomes['configuration_immutable'] = 1
    if expected_symbols:
        metadata['post_system_symbols_sha256'] = digest(symbols)
        if metadata['post_system_symbols_sha256'] != expected_symbols or has_links(symbols):
            outcomes['system_symbols_immutable'] = 1
    if expected_notice:
        metadata['post_layout_source_notice_sha256'] = (
            input_digest(notice) if notice.is_file() else None)
        if metadata['post_layout_source_notice_sha256'] != expected_notice or has_links(notice):
            outcomes['layout_source_notice_immutable'] = 1
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
    parent = verification_root()
    if not parent.exists():
        return 0
    runs = sorted(path for path in parent.iterdir() if path.is_dir() and path.name != 'lane')
    for run in runs:
        result = run / 'result.json'
        if not result.exists():
            raise RuntimeError(f'{run} has no result; inspect it before cleaning')
        state = receipt_state(result)
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
    snapshot = arguments.snapshot or Path((verification_root() / 'latest').read_text().strip())
    print((snapshot / 'result.json').read_text(), end='')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, subprocess.CalledProcessError, RuntimeError) as error:
        print(f'verification snapshot: {error}', file=sys.stderr)
        sys.exit(2)
