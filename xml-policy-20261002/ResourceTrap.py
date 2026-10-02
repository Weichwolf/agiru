from concurrent.futures import ThreadPoolExecutor
import errno
import json
import os
from pathlib import Path
import subprocess
import tempfile
import threading
import time

root = Path(__file__).resolve().parent
rows = []
for mode in ('unsafe', 'guarded', 'prohibit'):
    with tempfile.TemporaryDirectory(prefix='agiru-xml-resource-trap-') as directory:
        fifo = Path(directory) / 'owned-entity.txt'
        os.mkfifo(fifo)
        stop = threading.Event()
        opened = threading.Event()

        def writer():
            while not stop.is_set():
                try:
                    descriptor = os.open(fifo, os.O_WRONLY | os.O_NONBLOCK)
                except OSError as error:
                    if error.errno != errno.ENXIO:
                        raise
                    stop.wait(0.005)
                    continue
                opened.set()
                try:
                    os.write(descriptor, b'AGIRU-OWNED-XML-POLICY-MARKER')
                finally:
                    os.close(descriptor)
                return

        with ThreadPoolExecutor(max_workers=1) as executor:
            future = executor.submit(writer)
            started = time.monotonic()
            try:
                result = subprocess.run([str(root / 'context-probe'), directory, mode], capture_output=True, text=True, timeout=5)
            finally:
                stop.set()
            future.result(timeout=1)
        assert result.returncode == 0, result.stderr
        observations = [json.loads(line) for line in result.stdout.splitlines()]
        assert len(observations) == 1
        row = {'mode': mode, 'external_resource_opened': opened.is_set(), 'elapsed_seconds': time.monotonic() - started,
               'exit': result.returncode, 'observation': observations[0]}
        assert row['external_resource_opened'] == (mode == 'unsafe'), row
        rows.append(row)
receipt = {'fixture_kind': 'owned FIFO; independent writer observes the reader opening the resource',
           'private_host_data_read': False, 'network_trap_proved': False, 'production_reader_fixed': False, 'rows': rows}
(root / 'resource-trap.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
