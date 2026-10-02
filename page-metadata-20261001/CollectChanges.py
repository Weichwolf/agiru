from pathlib import Path

template = Path(__file__).resolve().parent.parent / 'object-options-20261001/CollectChanges.py'
text = template.read_text()
pairs = [
    ("expected = {'include/platform/ObjectOptions.h', 'src/gen/CodeunitWriter.cpp',\n            'src/gen/TableWriter.cpp', 'test/gate/NativeObjectGate.cpp',\n            'test/gate/GenTableBindingGate.cpp', 'test/toolchain.py'}",
     "expected = {'include/platform/PageMetadata.h', 'src/rt/Storage.cpp', 'test/gate/NativeObjectGate.cpp'}"),
    ("if len(generated) != 26 or 'apps/absent/absent/Types.h' not in generated:", "if generated:"),
]
for before, after in pairs:
    if text.count(before) != 1:
        raise SystemExit('predecessor changes collector changed: ' + before)
    text = text.replace(before, after)
exec(compile(text, __file__, 'exec'), globals())
