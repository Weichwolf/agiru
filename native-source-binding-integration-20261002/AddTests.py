from pathlib import Path
import difflib

task = Path(__file__).resolve().parent
root = task / 'source'
main = task.parent.parent
patches = []

def add(path, text):
    assert not (root / path).exists(), path
    patches.append('*** Add File: ' + str(root / path) + '\n' +
                   ''.join('+' + line + '\n' for line in text.splitlines()))

system = main / 'build/compiler-llvm-20261001/system_symbols/src/Virtual Tables/PageTableField.Table.al'
add('test/native-source/symbols/src/PageTableField.al', system.read_text())
add('test/native-source/symbols/NavxManifest.xml',
    '<Package><App Name="System" Publisher="Microsoft"/></Package>\n')
add('test/native-source/symbols/SymbolReference.json', '{}\n')
add('test/native-source/apps.json', '{"apps":[{"name":"fixture","source":"fixture","depends":[]}]}\n')
add('test/native-source/scope.json',
    '{"include":["System.Tooling"],"exclude":[],"area_exclude":[],"area_exclude_suffix":[],"product_exclude":[]}\n')
add('test/native-source/al/fixture/PageFieldsSelectionList.Page.al',
    (Path('/home/cosmo/Git/BCApps/src/Layers/W1/BaseApp/Modules/System/PageDesigner/PageFieldsSelectionList.Page.al')).read_text())
add('test/native-source/Consumer.cpp.in', r'''#include "fixture/system/tooling/page/PageFieldsSelectionList.h"
#include "meta/Ids.h"
#include "runtime/Page.h"
#include "type/Option.h"

int main() {
  using P = agiru::System::Tooling::PageFieldsSelectionList_Page;
  P page;
  page.Rec.Caption = "caption from the source record";
  page.Rec.Type = agiru::platform::PageTableFieldType::Text;
  if (page.Rec.Type.Ordinal() != 31488) { return 1; }
  const auto &definition = agiru::PageTraits<P>::kPage;
  if (definition.source != agiru::TableId{2000000171}) { return 2; }
  if (definition.layout[0].children[0].children[0].field != agiru::FieldNo{5}) { return 3; }
  for (const auto &trigger : agiru::PageTraits<P>::kControlTriggers) {
    if (trigger.sourceText != nullptr &&
        (page.*trigger.sourceText)().View() != "caption from the source record") { return 4; }
  }
  return page.Rec.Caption.View() != "caption from the source record";
}
''')

classes = r'''

class SystemSourceBindingGate(unittest.TestCase):
    def setUp(self):
        self.root = SCRIPT.parents[1]
        self.build = (self.root / Path(os.environ.get('B', 'build'))).resolve()
        self.fixtures = self.root / 'test/native-source'
        cache = (self.build / 'CMakeCache.txt').read_text()
        compiler = re.search(r'^CMAKE_CXX_COMPILER:[^=]+=(.+)$', cache, re.M)
        self.assertIsNotNone(compiler)
        self.compiler = compiler[1]

    def generate(self, folder, symbols_root=None, extra=()):
        output = folder / 'generated'
        command = [str(self.build / 'agirutc'), str(self.fixtures / 'al'),
                   str(self.fixtures / 'apps.json'), str(output)]
        if symbols_root is not None:
            command += ['--system-symbols', str(symbols_root)]
        result = subprocess.run(command + list(extra), cwd=self.root, capture_output=True,
                                text=True, timeout=30)
        return result, output

    def compile(self, output, source, executable=None):
        command = [self.compiler, '-std=c++23', '-stdlib=libc++',
            '-Wall', '-Wextra', '-Wpedantic', '-Werror', f'-I{self.root / "include"}',
            *(f'-I{path}' for path in (output, output / 'fixture', output / 'shared', output / 'absent'))]
        if executable is None:
            command += ['-fsyntax-only', str(source)]
        else:
            command += ['--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
                *(str(path) for path in sorted(output.rglob('*.cpp'))), str(source),
                f'-L{self.build}', f'-Wl,-rpath,{self.build}', '-lagiru_rt', '-lagiru_al',
                '-lagiru_net', '-lagiru_db', '-o', str(executable)]
        return subprocess.run(command, cwd=self.root, capture_output=True, text=True, timeout=60)

    def test_original_page_uses_complete_system_source(self):
        with tempfile.TemporaryDirectory() as temp:
            folder = Path(temp)
            result, output = self.generate(folder, self.fixtures / 'symbols')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('SYSTEM TABLES: 1 parsed from 1 AL files', result.stdout)
            self.assertIn('SYSTEM TABLES: 1 bound, 0 unbound', result.stdout)
            body = output / 'fixture/system/tooling/page/PageFieldsSelectionList.cpp'
            self.assertNotIn('return Format(Caption);', body.read_text())
            definition = body.with_suffix('.def.cpp').read_text()
            for name in ('Page ID', 'Index', 'Type', 'Length', 'Caption', 'Status',
                         'IsTableField', 'Scope', 'Tooltip', 'FieldKind', 'Name',
                         'Field ID', 'Table No', 'Description', 'Table Field Id'):
                self.assertIn('native field declaration mismatch: Page Table Field.' + name,
                              definition)
            consumer = folder / 'Consumer.cpp'
            consumer.write_text((self.fixtures / 'Consumer.cpp.in').read_text())
            executable = folder / 'consumer'
            compiled = self.compile(output, consumer, executable)
            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
            ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
            self.assertEqual(ran.returncode, 0, ran.stdout + ran.stderr)

    def test_native_contracts_refuse_wrong_but_existing_declarations(self):
        changes = (
            ('field(5; Caption; Text[80])', 'field(5; Caption; Text[81])', 'native field declaration mismatch'),
            ('field(5; Caption; Text[80])', 'field(5; Caption; Code[80])', 'native field declaration mismatch'),
            ('field(5; Caption; Text[80])', 'field(5; DifferentCaption; Text[80])', 'native field declaration mismatch'),
            ('field(5; Caption; Text[80])', 'field(4; Caption; Text[80])', 'native field declaration mismatch'),
            ('OptionMembers = New,Ready,Placed;', 'OptionMembers = New,Placed,Ready;', 'native field declaration mismatch'),
            ('4912, 4988,', '4913, 4988,', 'native field declaration mismatch'),
            ('key(pk; "Page ID", Index)', 'key(pk; Index, "Page ID")', 'native key declaration mismatch'),
            ('key(pk; "Page ID", Index)\n        {', 'key(pk; "Page ID", Index)\n        {\n            Clustered = false;', 'native key declaration mismatch'),
            ('DataPerCompany = false;', 'DataPerCompany = true;', 'native company scope mismatch'),
        )
        original = (self.fixtures / 'symbols/src/PageTableField.al').read_text()
        for old, new, diagnostic in changes:
            with self.subTest(old=old, new=new), tempfile.TemporaryDirectory() as temp:
                folder = Path(temp)
                system = folder / 'symbols'
                shutil.copytree(self.fixtures / 'symbols', system)
                self.assertEqual(original.count(old), 1)
                (system / 'src/PageTableField.al').write_text(original.replace(old, new))
                result, output = self.generate(folder, system)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                definition = output / 'fixture/system/tooling/page/PageFieldsSelectionList.def.cpp'
                compiled = self.compile(output, definition)
                self.assertNotEqual(compiled.returncode, 0, 'native contract accepted the mutant')
                self.assertIn(diagnostic, compiled.stderr)

    def test_invalid_system_inputs_and_arguments_refuse(self):
        for condition in ('missing', 'symlink', 'duplicate', 'unbound_duplicate', 'parse_error', 'extra_argument'):
            with self.subTest(condition=condition), tempfile.TemporaryDirectory() as temp:
                folder = Path(temp)
                system = folder / 'symbols'
                shutil.copytree(self.fixtures / 'symbols', system)
                extra = ()
                if condition == 'missing':
                    (system / 'NavxManifest.xml').unlink()
                elif condition == 'symlink':
                    (system / 'src/link.al').symlink_to(system / 'src/PageTableField.al')
                elif condition == 'duplicate':
                    shutil.copyfile(system / 'src/PageTableField.al', system / 'src/Another.al')
                elif condition == 'unbound_duplicate':
                    text = 'table 2000000999 Unknown { fields { field(1; ID; Integer) {} } }'
                    for name in ('A', 'B'):
                        (system / 'src' / (name + '.al')).write_text(text)
                elif condition == 'parse_error':
                    (system / 'src/Broken.al').write_text('table 2000000999 Broken { fields {')
                else:
                    extra = ('--ignored',)
                result, output = self.generate(folder, system, extra)
                self.assertNotEqual(result.returncode, 0)
                self.assertFalse(output.exists(), 'invalid input changed the output tree')

    def test_unbound_system_identities_remain_reported(self):
        with tempfile.TemporaryDirectory() as temp:
            folder = Path(temp)
            system = folder / 'symbols'
            shutil.copytree(self.fixtures / 'symbols', system)
            (system / 'src/Unknown.al').write_text('table 2000000999 Unknown {}')
            (system / 'src/Namespace.al').write_text('namespace System.Fixture;')
            result, _ = self.generate(folder, system)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('SYSTEM TABLES: 2 parsed from 3 AL files', result.stdout)
            self.assertIn('SYSTEM TABLE UNBOUND: Unknown ID 2000000999 source src/Unknown.al', result.stdout)
            self.assertIn('SYSTEM SOURCE UNBOUND: src/Namespace.al', result.stdout)
            self.assertIn('SYSTEM TABLES: 1 bound, 1 unbound', result.stdout)
'''

path = root / 'test/toolchain.py'
before = path.read_text()
anchor = "\n\nif __name__ == '__main__':\n"
assert before.count(anchor) == 1
after = before.replace(anchor, classes + anchor)
diff = ''.join(difflib.unified_diff(before.splitlines(True), after.splitlines(True), n=3))
lines = ['@@' if line.startswith('@@') else line for line in diff.splitlines()[2:]]
patches.append('*** Update File: ' + str(path) + '\n' + '\n'.join(lines) + '\n')
print('*** Begin Patch\n' + ''.join(patches) + '*** End Patch')
