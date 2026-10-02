from pathlib import Path
import json
import subprocess
import time

root = Path(__file__).resolve().parent
source = root / 'source'
artifacts = root / 'artifacts'
receipts = []
for compiler, tag in (('clang++-19', 'clang'), ('g++-14', 'gcc')):
    for name, relative in (
            ('invite-regenerated', 'base/accountant_portal/codeunit/InviteExternalAccountant.cpp'),
            ('webhook-gap', 'base/api/webhooks/codeunit/APIWebhookNotificationSend.cpp')):
        command = [compiler, '-std=c++23', '-O2', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                   '-fsyntax-only', '-I' + str(source / 'include')]
        command += ['-I' + str(path) for path in (source / 'apps').iterdir() if path.is_dir()]
        command += [str(source / 'apps' / relative)]
        start = time.monotonic()
        result = subprocess.run(command, text=True, capture_output=True, timeout=60)
        elapsed = time.monotonic() - start
        (artifacts / f'{name}-{tag}.log').write_text(result.stdout + result.stderr)
        receipts.append({'name': name, 'compiler': compiler, 'no_pch': True,
                         'command': command, 'exit': result.returncode, 'elapsed_seconds': elapsed})
        print(name, compiler, result.returncode, flush=True)
(artifacts / 'regenerated-body-commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
