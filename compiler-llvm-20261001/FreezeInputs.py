import importlib.util
import json
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('verify_snapshot', root / 'source/scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)
bc = Path('/home/cosmo/Git/BCApps/src')
symbols = Path('/home/cosmo/Git/agiru-worktrees/goal-20260928/build/symbol-package-proof/frozen.tkWmyH')
assert not (root / 'bc_source').exists() and not (root / 'system_symbols').exists()
revision = subprocess.check_output(['git', '-C', str(bc), 'rev-parse', 'HEAD'], text=True).strip()
assert revision == 'a9ea4d84534cebba852c44bf0f841c2ea149de4e'
receipt = {'bc_revision': revision,
           'bc_source_sha256': verify.freeze_input(bc, root / 'bc_source', 'BCApps'),
           'system_symbols_sha256': verify.freeze_input(symbols, root / 'system_symbols', 'System symbols')}
(root / 'artifacts/inputs.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt))
