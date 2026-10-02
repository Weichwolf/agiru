import difflib
from pathlib import Path

task = Path(__file__).resolve().parent
root = task / 'source'
prototype = task.parent / 'native-source-binding-integration-20261002/source'
patches = []

def replaced(text, old, new):
    assert text.count(old) == 1, old[:120]
    return text.replace(old, new, 1)

def edit(path, after):
    before = (root / path).read_text() if (root / path).exists() else None
    assert before != after, path
    if before is None:
        patches.append('*** Add File: ' + str(root / path) + '\n' +
                       '\n'.join('+' + line for line in after.splitlines()) + '\n')
    else:
        diff = ''.join(difflib.unified_diff(before.splitlines(True), after.splitlines(True), n=3))
        lines = ['@@' if line.startswith('@@') else line for line in diff.splitlines()[2:]]
        patches.append('*** Update File: ' + str(root / path) + '\n' + '\n'.join(lines) + '\n')

for path in ['Makefile', 'src/tc/Main.cpp', 'src/gen/PageWriter.cpp', 'src/gen/TableWriter.cpp',
             'src/gen/TableWriter.h', 'test/gate/GenNativeBindingGate.cpp']:
    edit(path, (prototype / path).read_text())

path = 'src/gen/CodeunitWriter.h'
text = (root / path).read_text()
text = replaced(text, '#include <set>\n', '#include <set>\n#include <span>\n')
text = replaced(text, '  std::vector<al::ProcedureDecl> procedureDeclarations;\n',
                      '  std::vector<al::ProcedureDecl> procedureDeclarations;\n  std::string declarationAssertions{};\n')
text = replaced(text, '[[nodiscard]] TableIndex PlatformTables();\n',
    '[[nodiscard]] bool NeedsNativeDefinition(const TableRef &binding);\n\n'
    '[[nodiscard]] TableIndex PlatformTables();\n'
    '[[nodiscard]] TableIndex PlatformTables(std::span<const al::TableObject> declarations);\n'
    '[[nodiscard]] FieldEnums PlatformFieldEnums(std::span<const al::TableObject> declarations,\n'
    '                                            const TableIndex &tables);\n')
edit(path, text)

path = 'src/gen/CodeunitWriter.cpp'
text = (root / path).read_text()
old = (prototype / path).read_text()
text = replaced(text, '#include "Scope.h"\n', '#include "Scope.h"\n#include "TableWriter.h"\n')
for first, last in [('std::string SourceIncludes(', '\ntemplate <typename Ahead'),
                    ('bool NeedsNativeDefinition(', '\nnamespace {\n\nvoid FaceReach')]:
    block = old[old.index(first):old.index(last, old.index(first))]
    start = text.index(first) if first in text else text.index('TableIndex PlatformTables() {')
    finish = text.index(last, start)
    text = text[:start] + block + text[finish:]
text = replaced(text, '    if (ref != nullptr && !ref->header.empty()) { ahead(ref->identifier); }\n',
    '    if (ref != nullptr && !ref->header.empty()) {\n'
    '      if (NeedsNativeDefinition(*ref)) {\n'
    '        reachElement(ref->header);\n'
    '      } else {\n'
    '        ahead(ref->identifier);\n'
    '      }\n'
    '    }\n')
assert 'FieldEnumerationOf(const Objects &objects' in text
edit(path, text)

for path in sorted((prototype / 'test/native-source').rglob('*')):
    if path.is_file():
        edit(str(path.relative_to(prototype)), path.read_text())

path = 'test/toolchain.py'
text = (root / path).read_text()
old = (prototype / path).read_text()
first = old.index('class SystemSourceBindingGate(unittest.TestCase):')
last = old.index("if __name__ == '__main__':", first)
text = replaced(text, "if __name__ == '__main__':", old[first:last] + "if __name__ == '__main__':")
first = old.index('    def test_verification_entrypoint_is_offline_and_read_only(self):')
last = old.index('\n\nclass ', first)
method = old[first:last]
assert method in text, 'preserve the already-promoted verification regression'
edit(path, text)
print('*** Begin Patch\n' + ''.join(patches) + '*** End Patch')
