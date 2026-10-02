from pathlib import Path
import hashlib
import json
import shutil

root = Path(__file__).resolve().parent
artifacts = root / 'artifacts'
archive = artifacts / 'before-native-local-guard'
manifest = json.loads((archive / 'manifest.json').read_text())
shutil.copy2(archive / 'manifest.json', archive / 'manifest-original-unvalidated.json')
for name in ('changes.json', 'prototype.patch'):
    shutil.copy2(artifacts / name, archive / name)
manifest['source_sha256'] = json.loads((archive / 'changes.json').read_text())['source_sha256']
manifest['files'] = [{'path': row['path'], 'sha256':
    hashlib.sha256((archive / row['path']).read_bytes()).hexdigest()} for row in manifest['files']]
manifest['changes_receipt_copy_corrected_after_collector_terminal'] = True
(archive / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(json.dumps({'archive_source_sha256': manifest['source_sha256']}), flush=True)
