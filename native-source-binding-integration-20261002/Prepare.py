from pathlib import Path
import difflib

task = Path(__file__).resolve().parent
root = task / 'source'
previous = task.parent / 'page-table-field-20261001/source'
patches = []

def replace(text, old, new):
    assert text.count(old) == 1, old[:120]
    return text.replace(old, new, 1)

def edit(path, transform):
    before = (root / path).read_text()
    after = transform(before)
    assert after != before, path
    diff = ''.join(difflib.unified_diff(before.splitlines(True), after.splitlines(True), n=3))
    lines = ['@@' if line.startswith('@@') else line for line in diff.splitlines()[2:]]
    patches.append('*** Update File: ' + str(root / path) + '\n' + '\n'.join(lines) + '\n')

edit('src/gen/CodeunitWriter.h', lambda text: replace(replace(replace(text,
    '#include <set>\n', '#include <set>\n#include <span>\n'),
    '  std::vector<al::ProcedureDecl> procedureDeclarations;\n',
    '  std::vector<al::ProcedureDecl> procedureDeclarations;\n  std::string declarationAssertions{};\n'),
    '[[nodiscard]] TableIndex PlatformTables();\n',
    '[[nodiscard]] bool NeedsNativeDefinition(const TableRef &binding);\n\n'
    '[[nodiscard]] TableIndex PlatformTables();\n'
    '[[nodiscard]] TableIndex PlatformTables(std::span<const al::TableObject> declarations);\n'
    '[[nodiscard]] FieldEnums PlatformFieldEnums(std::span<const al::TableObject> declarations,\n'
    '                                             const TableIndex &tables);\n'))
edit('src/gen/TableWriter.h', lambda text: replace(text,
    'std::string VariableIdentifier',
    '[[nodiscard]] std::string NativeTableAssertions(const al::TableObject &table,\n'
    '                                                 const TableRef &binding);\n\n'
    'std::string VariableIdentifier'))

prototype = (previous / 'src/gen/TableWriter.cpp').read_text()
first = prototype.index('namespace {\n\nstruct KeyFlags {')
last = prototype.index('\nnamespace {\n\ntemplate <typename Index>', first)
contracts = prototype[first:last]
contracts = replace(contracts,
    '  al::TableObject table = declared;\n  al::EnsurePrimaryKey(table);\n',
    '  const al::TableObject &table = declared;\n'
    '  if (table.keys.empty()) {\n'
    '    return "static_assert(false, " +\n'
    '           Literal("native implicit primary key is unrepresented: " + table.name) + ");\\n";\n'
    '  }\n')
contracts = replace(contracts,
    '  const bool company = perCompany == nullptr || LowerKey(perCompany->text) != "false";\n',
    '  if (perCompany != nullptr && LowerKey(perCompany->text) != "true" &&\n'
    '      LowerKey(perCompany->text) != "false") {\n'
    '    throw std::runtime_error("invalid native DataPerCompany: " + table.name);\n'
    '  }\n'
    '  const bool company = perCompany == nullptr || LowerKey(perCompany->text) == "true";\n')
def table_writer(text):
    for header in ('charconv', 'format', 'span', 'stdexcept', 'system_error'):
        text = replace(text, '#include <algorithm>\n', '#include <algorithm>\n#include <' + header + '>\n')
    return replace(text, '\nnamespace {\n\ntemplate <typename Index>', '\n' + contracts + '\nnamespace {\n\ntemplate <typename Index>')
edit('src/gen/TableWriter.cpp', table_writer)

prototype = (previous / 'src/gen/CodeunitWriter.cpp').read_text()
first = prototype.index('TableIndex PlatformTables(std::span<const al::TableObject> declarations)')
last = prototype.index('\nnamespace {\n\nvoid FaceReach', first)
registry = prototype[first:last]
registry = replace(registry,
    '  TableIndex tables;\n  for (const al::TableObject &table : declarations) {',
    '  TableIndex tables;\n  std::set<int> ids;\n  std::set<std::string> names;\n'
    '  for (const al::TableObject &table : declarations) {\n'
    '    if (!ids.insert(table.id).second || !names.insert(LowerKey(table.name)).second) {\n'
    '      throw std::runtime_error("duplicate System table declaration: " + table.name);\n'
    '    }')
registry = replace(registry,
    'FieldEnums PlatformFieldEnums(std::span<const al::TableObject> declarations) {\n'
    '  const FieldEnums bindings = PlatformFieldEnums();\n'
    '  const TableIndex tables = PlatformTables(declarations);\n',
    'FieldEnums PlatformFieldEnums(std::span<const al::TableObject> declarations,\n'
    '                              const TableIndex &tables) {\n'
    '  const FieldEnums bindings = PlatformFieldEnums();\n')
def codeunit(text):
    text = replace(text, '#include "Token.h"\n', '#include "Token.h"\n#include "TableWriter.h"\n')
    text = replace(text, 'TableIndex PlatformTables() {',
        'bool NeedsNativeDefinition(const TableRef &binding) {\n'
        '  return Unprefixed(binding.identifier).starts_with("platform::");\n}\n\n'
        'TableIndex PlatformTables() {')
    text = replace(text, '                       .header = "platform/" + Identifier(name) + ".h",\n',
        '                       .header = "platform/" + Identifier(name) + ".h",\n'
        '                       .id = std::stoi(std::string(number)),\n')
    text = replace(text, '  add("Page Metadata", "2000000138");\n',
        '  add("Page Metadata", "2000000138");\n  add("Page Table Field", "2000000171");\n')
    text = replace(text, '  enums["page metadata"]["pagetype"]',
        '  enums["page table field"]["type"] = "::agiru::platform::PageTableFieldType";\n'
        '  enums["page table field"]["status"] = "::agiru::platform::PageTableFieldStatus";\n'
        '  enums["page table field"]["scope"] = "::agiru::platform::PageTableFieldScope";\n'
        '  enums["page table field"]["fieldkind"] = "::agiru::platform::PageTableFieldKind";\n'
        '  enums["2000000171"] = enums["page table field"];\n'
        '  enums["page metadata"]["pagetype"]')
    text = replace(text, '\nnamespace {\n\nvoid FaceReach', '\n' + registry + '\nnamespace {\n\nvoid FaceReach')
    text = replace(text,
        '    if (ref != nullptr && !ref->header.empty()) { ahead(ref->identifier); }\n',
        '    if (ref != nullptr && !ref->header.empty()) {\n'
        '      if (NeedsNativeDefinition(*ref)) { reachElement(ref->header); }\n'
        '      else { ahead(ref->identifier); }\n'
        '    }\n')
    first = text.index('std::string SourceIncludes(')
    last = text.index('\ntemplate <typename Ahead', first)
    source = text[first:last]
    source = replace(source, '  std::set<std::string> headers;\n',
        '  std::set<std::string> headers;\n  std::map<std::string, const TableRef *> contracts;\n')
    source = replace(source,
        '  const auto reach = [&](const al::VarDecl &declared) {\n',
        '  const auto note = [&](const TableRef *ref) {\n'
        '    if (ref == nullptr) { return; }\n'
        '    if (!ref->header.empty()) { headers.insert(ref->header); }\n'
        '    if (!ref->declarationAssertions.empty()) { contracts.emplace(ref->identifier, ref); }\n'
        '  };\n'
        '  const auto reach = [&](const al::VarDecl &declared) {\n')
    source = replace(source,
        '    if (ref != nullptr && !ref->header.empty()) { headers.insert(ref->header); }\n',
        '    note(ref);\n')
    source = replace(source, '  std::string out;\n',
        '  const auto implicit = objects.tables.find(LowerKey(TableNoOf(unit)));\n'
        '  if (implicit != objects.tables.end()) { note(&implicit->second); }\n'
        '  std::string out;\n')
    source = replace(source, '  return out;\n',
        '  for (const auto &[name, ref] : contracts) {\n'
        '    static_cast<void>(name);\n    out += ref->declarationAssertions;\n  }\n'
        '  return out;\n')
    text = text[:first] + source + text[last:]
    return text
edit('src/gen/CodeunitWriter.cpp', codeunit)

def pages(text):
    text = replace(text, '  if (complete || declared.temporary) {',
        '  if (complete || declared.temporary || NeedsNativeDefinition(*ref)) {')
    text = replace(text, '  std::string out = "namespace " + space + " {\\n\\n";\n',
        '  std::string out;\n'
        '  if (source != nullptr) {\n'
        '    const auto bound = objects.tables.find(std::to_string(source->id));\n'
        '    if (bound != objects.tables.end()) { out += bound->second.declarationAssertions; }\n'
        '  }\n'
        '  out += "namespace " + space + " {\\n\\n";\n')
    return text
edit('src/gen/PageWriter.cpp', pages)

prototype = (previous / 'src/tc/Main.cpp').read_text()
first = prototype.index('std::vector<std::filesystem::path> SystemSources(')
last = prototype.index('\nusing OptionsInScope', first)
loader = prototype[first:last]
loader = replace(loader, 'agiru::gen::PlatformFieldEnums(tables.objects);',
                         'agiru::gen::PlatformFieldEnums(tables.objects, objects.tables);')
loader = replace(loader, '      continue;\n    }\n    ++bound;',
    '      else {\n'
    '        std::println("SYSTEM TABLE UNBOUND: {} ID {} source {}",\n'
    '                     table.name, table.id, tables.paths.at(&table - tables.objects.data()));\n'
    '      }\n'
    '      continue;\n    }\n    ++bound;')
def main(text):
    text = replace(text, '#include "TableWriter.h"\n', '#include "TableWriter.h"\n#include "Token.h"\n')
    text = replace(text, '  std::filesystem::path apps;\n};\n',
        '  std::filesystem::path apps;\n  std::filesystem::path systemSymbols;\n};\n')
    text = replace(text, 'void RefreshTableIndex(const Tables &tables,', loader + '\nvoid RefreshTableIndex(const Tables &tables,')
    text = replace(text, '  ClaimOutput(job.output);\n',
        '  const Tables systemTables = ReadSystemTables(job.systemSymbols);\n'
        '  ClaimOutput(job.output);\n')
    text = replace(text,
        '  objects.tables = agiru::gen::PlatformTables();\n'
        '  objects.fieldEnums = agiru::gen::PlatformFieldEnums();\n',
        '  IndexSystemTables(systemTables, objects, everyTable);\n')
    text = replace(text, '  if (arguments.size() < 3) {\n',
        '  const bool plain = arguments.size() == 3 || arguments.size() == 4;\n'
        '  const bool symbols = arguments.size() == 6 &&\n'
        '                       std::string_view(arguments[4]) == "--system-symbols" &&\n'
        '                       !std::string_view(arguments[5]).empty();\n'
        '  if (!plain && !symbols) {\n')
    text = replace(text,
        '    std::fputs("agirutc <bcapps-src-root> <apps.json> [<output-root>]\\n", stderr);\n',
        '    std::fputs("agirutc <bcapps-src-root> <apps.json> [<output-root>] "\n'
        '               "[--system-symbols <root>]\\n", stderr);\n')
    text = replace(text, '                    .apps = std::filesystem::path(arguments[2])});\n',
        '                    .apps = std::filesystem::path(arguments[2]),\n'
        '                    .systemSymbols = symbols ? std::filesystem::path(arguments[5])\n'
        '                                             : std::filesystem::path{}});\n')
    return text
edit('src/tc/Main.cpp', main)

def make(text):
    return replace(text,
        '\t@$(B)/agirutc $${AGIRU_BC_SOURCE:-$$HOME/Git/BCApps/src} $(SELF)/apps.json $(SELF)/apps\n',
        '\t@if [ -n "$${AGIRU_SYSTEM_SYMBOLS:-}" ]; then \\\n'
        '\t  python3 $(SELF)/scripts/fetch_symbols.py --verify "$$AGIRU_SYSTEM_SYMBOLS" && \\\n'
        '\t  $(B)/agirutc "$${AGIRU_BC_SOURCE:-$$HOME/Git/BCApps/src}" $(SELF)/apps.json $(SELF)/apps \\\n'
        '\t    --system-symbols "$$AGIRU_SYSTEM_SYMBOLS"; \\\n'
        '\telse \\\n'
        '\t  $(B)/agirutc "$${AGIRU_BC_SOURCE:-$$HOME/Git/BCApps/src}" $(SELF)/apps.json $(SELF)/apps; \\\n'
        '\tfi\n')
edit('Makefile', make)
edit('scripts/fetch_symbols.py', lambda text: replace(replace(text,
    '    arguments = parser.parse_args()\n',
    '    parser.add_argument("--verify", type=pathlib.Path,\n'
    '                        help="verify an existing package without downloading or writing")\n'
    '    arguments = parser.parse_args()\n'),
    '    version = (ROOT / "BC_VERSION").read_text().strip()\n',
    '    if arguments.verify is not None:\n'
    '        ledger = verify_package(arguments.verify)\n'
    '        print(json.dumps(ledger, sort_keys=True))\n'
    '        return\n'
    '    version = (ROOT / "BC_VERSION").read_text().strip()\n'))
print('*** Begin Patch\n' + ''.join(patches) + '*** End Patch')
