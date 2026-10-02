from pathlib import Path
import sys

helper = Path(__file__).resolve().parent.parent / 'page-source-binding-20261001/Regenerate.py'
code = helper.read_text()
anchor = "artifacts = root / 'artifacts'"
assert code.count(anchor) == 1
code = code.replace(anchor, anchor + " / label\nartifacts.mkdir(exist_ok=True)")
exec(compile(code, str(helper), 'exec'),
     {'__file__': __file__, '__name__': '__main__', 'label': sys.argv[1]})
