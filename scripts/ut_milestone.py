#!/usr/bin/env python3
"""Run every source-counted UT codeunit in an isolated, disposable database."""
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import tempfile
import threading
import time
from urllib.parse import urlsplit, urlunsplit

from ut_manifest import scan
from ut_results import aggregate


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DSN = 'postgresql://agiru:agiru@localhost:5433/agiru_seeded'
TIMEOUT_SECONDS = 900
CLEANUP_SECONDS = 30


def maintenance_dsn(dsn):
    if dsn.startswith(('postgresql://', 'postgres://')):
        parsed = urlsplit(dsn)
        if not parsed.path or parsed.path == '/':
            raise ValueError('the template connection must name a database')
        return urlunsplit(parsed._replace(path='/postgres'))
    match = re.search(r"\bdbname\s*=\s*(?:'(?:\\.|[^'])*'|[^\s]+)", dsn)
    if match is None:
        raise ValueError('the template connection must name a database')
    return dsn[:match.start()] + 'dbname=postgres' + dsn[match.end():]


def source_revision(path):
    result = subprocess.run(['git', '-C', str(path), 'rev-parse', 'HEAD'],
                            capture_output=True, text=True, check=False)
    return result.stdout.strip() if result.returncode == 0 else None


def file_sha256(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def source_sha256(manifest, tests_root):
    digest = hashlib.sha256()
    for entry in manifest:
        source = Path(entry['source'])
        digest.update(str(source.relative_to(tests_root)).encode())
        digest.update(b'\0')
        digest.update(bytes.fromhex(file_sha256(source)))
    return digest.hexdigest()


def seed_snapshot(dsn):
    query = ("SELECT (SELECT oid FROM pg_database WHERE datname = current_database()), "
             "pg_database_size(current_database())")
    try:
        result = subprocess.run(['psql', '-XAt', '-v', 'ON_ERROR_STOP=1', '-F', '|',
                                 '-d', dsn, '-c', query], capture_output=True, text=True,
                                check=False, timeout=10)
    except (OSError, subprocess.TimeoutExpired):
        return None
    if result.returncode != 0:
        return None
    parts = result.stdout.strip().split('|')
    if len(parts) != 2 or not all(part.isdecimal() for part in parts):
        return None
    return {'database_oid': int(parts[0]), 'size_bytes': int(parts[1])}


def require_current_image():
    try:
        result = subprocess.run(['ninja', '-C', str(ROOT / 'build'), '-n', 'agiru'],
                                capture_output=True, text=True, check=False, timeout=30)
    except (OSError, subprocess.TimeoutExpired) as error:
        raise ValueError(f'cannot inspect the linked image: {error}') from error
    if result.returncode != 0 or not result.stdout.rstrip().endswith('ninja: no work to do.'):
        raise ValueError('agiru or a linked library is stale; run make or verify a frozen snapshot')


def write_json(path, value):
    replacement = path.with_suffix(path.suffix + '.pending')
    replacement.write_text(json.dumps(value, indent=2, sort_keys=True) + '\n')
    replacement.replace(path)


def run_one(entry, parts, dsn, maintenance, work_date, timeout, stop, active, lock):
    stem = parts / str(entry['id'])
    scratch = f'agiru_ut_{os.getpid()}_{entry["id"]}'
    log_path = stem.with_suffix('.log')
    command = [str(ROOT / 'build/agiru'), 'run-tests', '--database', dsn, '--fresh',
               '--scratch', scratch, '--codeunit', entry['name'],
               '--results-jsonl', str(stem.with_suffix('.jsonl'))]
    if work_date:
        command.extend(['--work-date', work_date])
    status = 130 if stop.is_set() else -1
    started = False
    with log_path.open('w') as log:
        if status != 130:
            try:
                with lock:
                    if stop.is_set():
                        status = 130
                    else:
                        process = subprocess.Popen(command, cwd=ROOT, stdout=log,
                                                   stderr=subprocess.STDOUT, start_new_session=True)
                        started = True
                        active[entry['id']] = process
                if started:
                    try:
                        status = process.wait(timeout=timeout)
                    except subprocess.TimeoutExpired:
                        terminate(process)
                        status = 124
                        log.write('INCOMPLETE: test runner timed out\n')
                    finally:
                        with lock:
                            active.pop(entry['id'], None)
                    if stop.is_set() and status != 124:
                        status = 130
            except OSError as error:
                status = 2
                log.write(f'INCOMPLETE: runner could not start or wait: {error}\n')
        if status == 130:
            log.write('INCOMPLETE: run interrupted before this codeunit completed\n')
        if started:
            for attempt in range(3):
                try:
                    cleanup = subprocess.run(
                        ['psql', '-X', '-v', 'ON_ERROR_STOP=1', '-d', maintenance, '-c',
                         f'DROP DATABASE IF EXISTS "{scratch}" WITH (FORCE)'],
                        stdout=log, stderr=subprocess.STDOUT, check=False, timeout=CLEANUP_SECONDS)
                    if cleanup.returncode == 0:
                        break
                    log.write(f'INCOMPLETE: scratch cleanup exited {cleanup.returncode}\n')
                except (OSError, subprocess.TimeoutExpired) as error:
                    log.write(f'INCOMPLETE: scratch cleanup error: {error}\n')
                if attempt < 2:
                    time.sleep(1)
            else:
                log.write('INCOMPLETE: scratch database cleanup failed\n')
                status = 2
    stem.with_suffix('.status').write_text(str(status) + '\n')


def terminate(process):
    if process.poll() is not None:
        return
    try:
        os.killpg(process.pid, signal.SIGTERM)
    except ProcessLookupError:
        return
    try:
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        process.wait()


def main(arguments):
    if len(arguments) not in (2, 3, 4):
        raise ValueError('usage: ut-milestone.sh output.log [workers] [master-dsn]')
    output = Path(arguments[1]).resolve()
    workers = int(arguments[2]) if len(arguments) >= 3 else 6
    if workers < 1:
        raise ValueError('workers must be positive')
    dsn = arguments[3] if len(arguments) == 4 else DEFAULT_DSN
    maintenance = maintenance_dsn(dsn)
    work_date = os.environ.get('AGIRU_WORK_DATE', '2028-01-25')
    timeout = int(os.environ.get('AGIRU_UT_TIMEOUT_SECONDS', str(TIMEOUT_SECONDS)))
    if timeout < 1:
        raise ValueError('UT timeout must be positive')
    require_current_image()
    source = Path(os.environ.get('AGIRU_BC_SOURCE', Path.home() / 'Git/BCApps/src'))
    tests_root = source / 'Layers/W1/Tests'
    manifest = scan(tests_root)
    output.parent.mkdir(parents=True, exist_ok=True)
    manifest_path = Path(str(output) + '.manifest.json')
    write_json(manifest_path, manifest)
    parts = Path(tempfile.mkdtemp(prefix=output.name + '.parts.', dir=output.parent))
    binary = ROOT / 'build/agiru'
    canonical_manifest = [{**entry, 'source': str(Path(entry['source']).relative_to(tests_root))}
                          for entry in manifest]
    manifest_digest = hashlib.sha256(json.dumps(canonical_manifest, sort_keys=True).encode()).hexdigest()
    images = [binary, *sorted(ROOT.joinpath('build').glob('libagiru_*.so'))]
    metadata = {
        'agiru_revision': source_revision(ROOT),
        'image_sha256': {path.name: file_sha256(path) for path in images},
        'source_revision': source_revision(source),
        'source_manifest_sha256': manifest_digest,
        'source_files_sha256': source_sha256(manifest, tests_root),
        'seed_database': urlsplit(dsn).path.lstrip('/') if '://' in dsn else None,
        'seed_identity': None,
        'seed_snapshot_hint': seed_snapshot(dsn),
        'work_date': work_date,
        'workers': workers,
        'timeout_seconds': timeout,
        'parts': str(parts),
        'command': ['agiru', 'run-tests', '--fresh', '--codeunit', '<source manifest>',
                    '--results-jsonl', '<per-codeunit artifact>'],
        'started_utc': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()),
    }
    metadata_path = Path(str(output) + '.run.json')
    write_json(metadata_path, metadata)
    stop = threading.Event()
    active = {}
    lock = threading.Lock()

    def request_stop(_signum, _frame):
        stop.set()

    signal.signal(signal.SIGINT, request_stop)
    signal.signal(signal.SIGTERM, request_stop)
    start = time.monotonic()
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        futures = [pool.submit(run_one, entry, parts, dsn, maintenance, work_date,
                               timeout, stop, active, lock) for entry in manifest]
        pending = set(futures)
        while pending:
            _, pending = concurrent.futures.wait(
                pending, timeout=0.25, return_when=concurrent.futures.FIRST_COMPLETED)
            if stop.is_set():
                with lock:
                    running = tuple(active.values())
                for process in running:
                    terminate(process)
                for future in pending:
                    future.cancel()
    failures = [future.exception() for future in futures
                if not future.cancelled() and future.exception() is not None]
    elapsed = int(time.monotonic() - start)
    status = aggregate(manifest, parts, output, workers, elapsed)
    if failures or stop.is_set():
        status = 1
    metadata['finished_utc'] = time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())
    metadata['status'] = status
    metadata['infrastructure_errors'] = [str(error) for error in failures]
    write_json(metadata_path, metadata)
    return status


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv))
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(f'UT milestone: {error}', file=sys.stderr)
        sys.exit(2)
