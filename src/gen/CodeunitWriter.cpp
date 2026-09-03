#include "CodeunitWriter.h"

#include "Ast.h"
#include "BodyWriter.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Scope.h"
#include "Token.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

bool IsPublisher(const al::ProcedureDecl &procedure) {
  return al::HasAttribute(procedure, "IntegrationEvent") ||
         al::HasAttribute(procedure, "BusinessEvent") ||
         al::HasAttribute(procedure, "InternalEvent");
}

bool NamesAnObject(const al::VarDecl &declared) {
  const std::string type = TypeName(declared.type);
  // AN OBJECT KIND WITH NO GENERATOR YET STILL NAMES AN OBJECT. `Report "X"`, `Query "X"`,
  // `XmlPort "X"` and `ControlAddIn "X"` resolve to nothing, which makes them `absent::X` -- the
  // same shape an untranslated table takes, and a refusal that names what it wanted (board:0034).
  return (type == "Record" || type == "Codeunit" || type == "Page" || type == "Report" ||
          type == "Query" || type == "XmlPort" || type == "ControlAddIn") &&
         !declared.subtype.empty();
}

// A type as it stands in a DECLARATION. `Record "X"` is the generated class, `Record "X" temporary`
// is that class with no database behind it, `Text[50]` carries its length, and everything else is
// the door's own name for the AL type.
const TableRef *Reach(const al::VarDecl &declared, const Objects &objects) {
  const std::string type = TypeName(declared.type);
  const TableIndex &index = type == "Codeunit" ? objects.codeunits
                            : type == "Page"   ? objects.pages
                                               : objects.tables;
  const auto found = index.find(LowerKey(declared.subtype));
  return found != index.end() ? &found->second : nullptr;
}

// AN INLINE `Option A,B,C` DECLARES ITS OWN MEMBERS AND HAS NO NAME, so the generator gives it
// one: the codeunit, the procedure it stands in, and the variable. A table does the same for its
// own inline options, and the alternative -- an Integer -- would compile and lose the vocabulary
// `Type::All` is written in.
std::string InterfaceType(const al::VarDecl &declared, const Objects &objects) {
  const auto found = objects.interfaces.find(LowerKey(declared.subtype));
  return (found != objects.interfaces.end() ? found->second.identifier
                                            : "absent::" + Identifier(declared.subtype)) +
         " *";
}

/// Whether a declaration names a type that is neither AL's nor an object -- a bare platform enum.
///
/// AL writes the platform's own enums as bare type names: `Verbosity: Verbosity`,
/// `DataClassification: DataClassification`. Emitting the word unchanged put it beside the door,
/// where nothing declares it.
/// A method whose leading arguments are FIELDS OF THE RECEIVER, and how many.
///
/// The body writer spells them against the receiver; this is the same table, because the gatherer
/// has to know what the writer will reach for.
constexpr std::size_t kEveryArgument = static_cast<std::size_t>(-1);

std::size_t FieldArguments(std::string_view method) {
  static const std::vector<std::pair<std::string_view, std::size_t>> kTakers{
      {"SetRange", 1},
      {"SetFilter", 1},
      {"TestField", 1},
      {"FieldError", 1},
      {"FieldCaption", 1},
      {"FieldName", 1},
      {"FieldNo", 1},
      {"Validate", 1},
      {"SetAscending", 1},
      {"CalcFields", kEveryArgument},
      {"CalcSums", kEveryArgument},
      {"SetCurrentKey", kEveryArgument},
      {"SetLoadFields", kEveryArgument},
      {"AddLoadFields", kEveryArgument},
  };
  for (const auto &[name, count] : kTakers) {
    if (LowerKey(std::string(name)) == LowerKey(std::string(method))) { return count; }
  }
  return 0;
}

bool NamesAbsentType(const al::VarDecl &declared) {
  // AN EMPTY TYPE IS NOT A TYPE. A procedure with no return has one, and calling it absent named a
  // struct with no name at all.
  return !declared.type.empty() && !IsAlTypeName(declared.type) && declared.subtype.empty() &&
         declared.members.empty() && declared.arguments.empty();
}

std::string ObjectType(const al::VarDecl &declared, const Objects &objects) {
  const TableRef *ref = Reach(declared, objects);
  // AN OBJECT THIS RUN DOES NOT HAVE IS NAMED AS ABSENT. It used to be emitted as a bare
  // identifier, which named nothing; and putting the stub beside the real objects shadowed the AL
  // TYPES, because the virtual table `Integer` and the AL type `Integer` are one word.
  if (ref == nullptr) { return "absent::" + Identifier(declared.subtype); }
  return declared.temporary ? "Temporary<" + ref->identifier + ">" : ref->identifier;
}

std::string
TypeOf(const al::VarDecl &declared, const Objects &objects, const std::string &owner = {});

/// A generic's element types, each read by the same function that read the generic.
std::string Generic(const std::string &type,
                    const std::vector<al::VarDecl> &arguments,
                    const Objects &objects) {
  std::string out = type + "<";
  for (std::size_t i = 0; i < arguments.size(); ++i) {
    if (i != 0) { out += ", "; }
    out += TypeOf(arguments[i], objects);
  }
  return out + ">";
}

std::string Element(const al::VarDecl &declared, const Objects &objects, const std::string &owner);

/// AN ARRAY WRAPS WHAT IT HOLDS, outermost dimension first, and it is indexed from ONE.
std::string TypeOf(const al::VarDecl &declared, const Objects &objects, const std::string &owner) {
  std::string inner = Element(declared, objects, owner);
  for (std::size_t i = declared.dimensions.size(); i > 0; --i) {
    std::string wrapped = "AlArray<";
    wrapped += inner;
    wrapped += ", ";
    wrapped += std::to_string(declared.dimensions[i - 1]);
    wrapped += ">";
    inner = std::move(wrapped);
  }
  return inner;
}

/// What a type is being read FOR: the canonical AL type name, and the object that declares it.
struct Named {
  std::string type;  ///< What `Element` canonicalised the AL type name to.
  std::string owner; ///< The enclosing object, which names an inline option's enumeration.
};

/// The half of the type map that carries a TEMPLATE ARGUMENT or a length: the headless page, an
/// inline option, a generic, an absent type and the sized strings.
std::string Parameterised(const al::VarDecl &declared, const Objects &objects, const Named &named) {
  const std::string &type = named.type;
  const std::string &owner = named.owner;
  // A PAGE VARIABLE NAMES ITS PAGE, and `TestPage` is a class template that takes it: AL's
  // `TestPage "Payment Journal"` is the headless page and not a page-shaped thing, so the argument
  // is what makes `PaymentJournal."No.".SetValue(...)` resolve to a control at all. A page this run
  // never saw leaves the template empty, the way an unresolved enumeration does.
  if (type == "TestPage" || type == "TestRequestPage") {
    if (declared.subtype.empty()) { return type + "<>"; }
    const auto found = objects.pages.find(LowerKey(declared.subtype));
    return found != objects.pages.end() ? type + "<" + found->second.identifier + ">" : type + "<>";
  }
  if (type == "Option") {
    return declared.members.empty() || owner.empty() ? "Option<>" : "Option<" + owner + ">";
  }
  // `List of [Text]` and `Dictionary of [Text, Integer]` carry their element types with them, AND
  // THOSE NEST: `Dictionary of [Integer, List of [Text]]` is one the BaseApp writes. An argument is
  // read by this same function, so an inner generic, an enum's subtype and a `Text[50]`'s length
  // all survive.
  if ((type == "List" || type == "Dictionary") && !declared.arguments.empty()) {
    return Generic(type, declared.arguments, objects);
  }

  // A NAME THAT IS NEITHER AN AL TYPE NOR AN OBJECT THIS RUN READ IS ABSENT. AL writes the
  // platform's own enums as bare type names -- `Verbosity: Verbosity` -- and emitting the word
  // unchanged put it beside the door, where nothing declares it.
  if (NamesAbsentType(declared)) { return "absent::" + Identifier(declared.type); }
  // A BARE `Text` IS UNBOUNDED, and it lands here as a length of zero -- which the string types
  // read as no limit rather than a limit of nothing. `Text[50]` carries its 50.
  if (type == "Code" || type == "Text") {
    return type + "<" + std::to_string(declared.length) + ">";
  }
  return type;
}

std::string Element(const al::VarDecl &declared, const Objects &objects, const std::string &owner) {
  const std::string type = TypeName(declared.type);
  // A `Page` VARIABLE IS THE PAGE, the way a `Record` variable is the table: AL writes
  // `SalesOrderPage.RunModal()` on an instance. Only `TestPage` is a template, because the headless
  // page is a DIFFERENT type that wraps the page rather than being one.
  if (NamesAnObject(declared)) { return ObjectType(declared, objects); }
  // AN ENUM VARIABLE NAMES ITS ENUMERATION, and without it `Enum` is a class template with no
  // arguments -- which is not a type at all. The index is the same one a table field uses.
  // A DotNet VARIABLE IS TYPED BY ITS SUBTYPE, and the subtype was being thrown away: every one of
  // the 7 117 declarations came out as the bare word `DotNet`, so the compiler said "unknown type
  // name 'DotNet'" 1 971 times and named none of the 577 classes actually wanted. A .NET class
  // cannot be papered over the way an enum ordinal can -- it is used by CALLING it, and no
  // placeholder carries 577 method sets -- so what this buys is not a compile, it is a MEASUREMENT:
  // the roots now name `dotnet::XmlDocument` and the classes can be ranked by what they stop.
  if (type == "DotNet") {
    return declared.subtype.empty() ? "dotnet::Unnamed" : "dotnet::" + Identifier(declared.subtype);
  }
  // AN ENUMERATION THIS RUN NEVER SAW BECOMES `Enum<>`, which carries the ordinal and names no
  // member. `Copilot Capability` exists in BCApps only as enumextensions of something that is not
  // there -- the platform declares it. Emitting `Enum<enums::CopilotCapability>` named a type
  // nobody declares and stopped the file; inventing ordinals from the extensions would be a wrong
  // number that looks like a right one. The transpiler names every unresolved enumeration in its
  // summary, so this is reported rather than swallowed.
  if (type == "Enum") {
    if (declared.subtype.empty()) { return "Enum<>"; }
    const auto found = objects.enums.find(LowerKey(declared.subtype));
    return found != objects.enums.end() ? "Enum<enums::" + found->second.identifier + ">"
                                        : "Enum<>";
  }
  // AN OPTION WITH NO MEMBERS IS AL'S OWN DECLARATION AND NOT A GAP. `procedure P(ChangeType:
  // Option)` takes any option value at all, and the BaseApp calls it with a member of some other
  // enumeration entirely. `Option<>` is that: the ordinal, without a vocabulary.
  // AN INTERFACE VARIABLE HOLDS A POINTER TO THE ABSTRACT CLASS. AL writes `I.Method()` and C++
  // writes `I->Method()`, which is the deviation board:0027 names: an interface variable IS a
  // handle to something else, and 822 signatures cannot be forwarded by a wrapper.
  if (type == "Interface") { return InterfaceType(declared, objects); }
  return Parameterised(declared, objects, Named{.type = type, .owner = owner});
}

// A PARAMETER MAY BE NAMED AFTER ITS TYPE, and AL writes it constantly: `Item: Record Item`,
// `SalesLine: Record "Sales Line"`. In C++ the name then hides the class and the declaration needs
// an elaborated specifier or a qualification. Qualified, because `class Item &Item` reads like a
// C++ puzzle and `agiru::app::Item &Item` reads like what it is.
std::string
OptionName(const std::string &unit, const std::string &within, const std::string &name) {
  return Identifier(unit) + Identifier(within) + Identifier(name);
}

/// The generated name for ONE inline option, disambiguated when an overload declares another.
///
/// TWO OVERLOADS MAY DECLARE TWO DIFFERENT ANONYMOUS OPTIONS UNDER ONE PARAMETER NAME.
/// `Cryptography Management` has five `GenerateHash`, three taking
/// `HashAlgorithmType: Option MD5,SHA1,...` and two taking
/// `HashAlgorithmType: Option HMACMD5,HMACSHA1,...` -- one generated name, two enumerations, which
/// the generator refused outright and which cost the whole codeunit and everything including it.
/// The FIRST MEMBER separates them, is derived from the declaration alone, and so comes out the
/// same at every call site without anybody carrying a table around.
std::string OptionNameOf(const std::string &owner,
                         const std::string &within,
                         const al::VarDecl &declared,
                         const std::vector<al::ProcedureDecl> &procedures) {
  std::string base = OptionName(owner, within, declared.name);
  if (declared.members.empty()) { return base; }
  const auto clashes = [&](const al::ProcedureDecl &procedure, const al::VarDecl &other) {
    return !other.members.empty() && other.members != declared.members &&
           OptionName(owner, procedure.name, other.name) == base;
  };
  for (const al::ProcedureDecl &procedure : procedures) {
    const bool found =
        std::ranges::any_of(procedure.parameters,
                            [&](const al::VarDecl &o) { return clashes(procedure, o); }) ||
        std::ranges::any_of(procedure.variables,
                            [&](const al::VarDecl &o) { return clashes(procedure, o); });
    if (found) { return base + EnumeratorName(declared.members.front()); }
  }
  return base;
}

// THE NAMESPACE IS DECIDED BY WHAT THE TYPE IS, and getting it wrong cost more than anything else
// in this tree: `agiru::app::RecordRef` was the FIRST diagnostic of 1 375 of 3 123 failing headers,
// measured 2026-09-02, because `LibraryAssert` writes `RecordRef: RecordRef` and one bad
// qualification in it stops every header that includes it. An AL object -- a Record or a Codeunit
// -- becomes a class in `agiru::app`; every other AL type is a DOOR type and lives in `agiru`.
/// Whether any identifier inside a type is hidden by a name in the object's own scope.
///
/// A TEMPLATE ARGUMENT IS HIDDEN JUST AS THE OUTER TYPE IS. `MatchBankPayments` declares a member
/// called `Code` and then `Dictionary of [Integer, List of [Code[35]]]`, where only the innermost
/// name collides. So every identifier is asked, and one that follows `::` is skipped -- it is
/// already qualified and belongs to whatever named it.
bool Hidden(const std::string &type, const std::set<std::string> &names) {
  for (std::size_t i = 0; i < type.size();) {
    if (std::isalpha(static_cast<unsigned char>(type[i])) == 0 && type[i] != '_') {
      ++i;
      continue;
    }
    std::size_t end = i;
    while (end < type.size() &&
           (std::isalnum(static_cast<unsigned char>(type[end])) != 0 || type[end] == '_')) {
      ++end;
    }
    const bool qualified = i >= 2 && type[i - 1] == ':' && type[i - 2] == ':';
    if (!qualified && names.contains(type.substr(i, end - i))) { return true; }
    i = end;
  }
  return false;
}

/// Every name declared in an object's own scope, because each of them hides a type of that name for
/// the whole class. `Any` declares `procedure Boolean(): Boolean` and the member wins from there
/// on.
std::set<std::string> Shadowing(const std::vector<al::VarDecl> &variables,
                                const std::vector<al::ProcedureDecl> &procedures,
                                const std::vector<al::LabelDecl> &labels) {
  std::set<std::string> names;
  for (const al::VarDecl &declared : variables) { names.insert(Identifier(declared.name)); }
  for (const al::ProcedureDecl &procedure : procedures) {
    names.insert(Identifier(procedure.name));
  }
  for (const al::LabelDecl &label : labels) { names.insert(Identifier(label.name)); }
  return names;
}

/// Qualifies every identifier in a type that a name in scope hides.
///
/// AN INNER IDENTIFIER NEEDS IT AS MUCH AS THE OUTER ONE. `GenJnlPostLine` declares a member `Code`
/// and a parameter of type `List of [Code[10]]`; prefixing the whole string gave
/// `agiru::List<Code<10>>` and the inner name was still the member. A bare identifier in a
/// generated type is always a DOOR type -- an object arrives already spelled `tables::X` -- so the
/// qualification is `agiru::` and nothing else has to be decided.
std::string Qualified(const std::string &type, const std::set<std::string> &names) {
  std::string out;
  for (std::size_t i = 0; i < type.size();) {
    if (std::isalpha(static_cast<unsigned char>(type[i])) == 0 && type[i] != '_') {
      out += type[i];
      ++i;
      continue;
    }
    std::size_t end = i;
    while (end < type.size() &&
           (std::isalnum(static_cast<unsigned char>(type[end])) != 0 || type[end] == '_')) {
      ++end;
    }
    const std::string word = type.substr(i, end - i);
    const bool qualified = i >= 2 && type[i - 1] == ':' && type[i - 2] == ':';
    if (!qualified && names.contains(word)) { out += "agiru::"; }
    out += word;
    i = end;
  }
  return out;
}

/// A PARAMETER NAME HIDES A TYPE NAME FOR EVERY PARAMETER AFTER IT, not only for its own.
/// `AccPeriodStartEnd(Date: Date; var StartDate: Date; ...)` declares a parameter called `Date`
/// and then two more of type `Date`, and C++ resolves the later ones to the parameter. So the
/// question is not whether THIS parameter is named after its type; it is whether ANY parameter in
/// the signature is named after this one's type.
std::string Signature(const al::VarDecl &declared,
                      const Objects &objects,
                      const std::set<std::string> &names,
                      const std::string &owner = {}) {
  std::string type = TypeOf(declared, objects, owner);
  if (Hidden(type, names)) { type = Qualified(type, names); }
  return type + (declared.byReference ? " &" : " ");
}

std::string Parameters(const al::ProcedureDecl &procedure,
                       const Objects &objects,
                       bool named,
                       const std::string &unit,
                       const std::set<std::string> &shadowed = {},
                       const std::vector<al::ProcedureDecl> &all = {}) {
  std::set<std::string> names = shadowed;
  for (const al::VarDecl &parameter : procedure.parameters) {
    names.insert(Identifier(parameter.name));
  }
  std::string out;
  for (std::size_t i = 0; i < procedure.parameters.size(); ++i) {
    if (i != 0) { out += ", "; }
    out += Signature(procedure.parameters[i],
                     objects,
                     names,
                     OptionNameOf(unit, procedure.name, procedure.parameters[i], all));
    if (named) { out += Identifier(procedure.parameters[i].name); }
  }
  return out;
}

/// Every inline option an object declares, as its own enumeration with its own traits.
///
/// A PAGE DECLARES THEM THE SAME WAY A CODEUNIT DOES, so the object's name and the namespace its
/// enumerations live in are arguments rather than assumptions.
std::string InlineOptionsIn(const std::string &owner,
                            const std::string &space,
                            const std::vector<al::VarDecl> &variables,
                            const std::vector<al::ProcedureDecl> &procedures) {
  std::string out;
  // AN OVERLOAD DECLARES THE SAME INLINE OPTION TWICE. `UserPermissionsImpl` has two
  // `GetEffectivePermission`, both taking `PermissionObjectType: Option "Table Data",...` -- one
  // AL parameter written twice, and the generated name is built from the codeunit, the procedure
  // and the parameter, so it is the same name both times. It is ONE enumeration; emitting it twice
  // is a redefinition.
  //
  // THE MEMBERS DECIDE, not the count. Two options that share a generated name and differ in what
  // they declare are two enumerations wearing one name, and that is a translation error rather
  // than something to pick a winner for.
  std::map<std::string, std::vector<std::string>> emitted;
  const auto declare = [&](const std::string &within, const al::VarDecl &declared) {
    if (TypeName(declared.type) != "Option" || declared.members.empty()) { return; }
    const std::string name = OptionNameOf(owner, within, declared, procedures);
    const auto seen = emitted.find(name);
    if (seen != emitted.end()) {
      if (seen->second != declared.members) {
        throw std::runtime_error("\"" + owner +
                                 "\" declares two different options under the name " + name);
      }
      return;
    }
    emitted.insert_or_assign(name, declared.members);
    const std::vector<std::string> names = EnumeratorNames(declared.members);
    out += "namespace agiru::app::" + space + " {\n\nenum class " + name + " : std::int32_t {\n";
    for (std::size_t i = 0; i < names.size(); ++i) {
      out += "  " + names[i] + " = " + std::to_string(i) + ",\n";
    }
    out += "};\n\n} // namespace agiru::app::" + space + "\n\n";
    out += "template <> struct agiru::OptionTraits<agiru::app::" + space + "::" + name + "> {\n";
    out += "  static constexpr std::array<EnumValueDef, " +
           std::to_string(declared.members.size()) + "> kValues{{\n";
    for (std::size_t i = 0; i < declared.members.size(); ++i) {
      out += "      EnumValueDef{.ordinal = " + std::to_string(i) +
             ", .name = " + Literal(declared.members[i]) +
             ", .caption = " + Literal(declared.members[i]) + "},\n";
    }
    out += "  }};\n};\n\n";
  };
  for (const al::VarDecl &declared : variables) { declare(std::string{}, declared); }
  for (const al::ProcedureDecl &procedure : procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { declare(procedure.name, declared); }
    for (const al::VarDecl &declared : procedure.variables) { declare(procedure.name, declared); }
  }
  return out;
}

std::string InlineOptions(const al::CodeunitObject &unit) {
  return InlineOptionsIn(unit.name, "codeunits", unit.variables, unit.procedures);
}

std::string Returns(const al::ProcedureDecl &procedure,
                    const Objects &objects,
                    const std::set<std::string> &shadowed = {}) {
  if (procedure.returnType.empty()) { return "void"; }
  const std::string type = TypeOf(procedure.returned, objects);
  return Hidden(type, shadowed) ? Qualified(type, shadowed) : type;
}

std::string Declaration(const al::ProcedureDecl &procedure,
                        const Objects &objects,
                        const std::string &unit,
                        const std::set<std::string> &shadowed = {},
                        const std::vector<al::ProcedureDecl> &all = {}) {
  std::set<std::string> hiding = shadowed;
  hiding.insert(Identifier(procedure.name));
  return "  " + Returns(procedure, objects, hiding) + " " + Identifier(procedure.name) + "(" +
         Parameters(procedure, objects, true, unit, hiding, all) + ");\n";
}

bool NamesAbsent(const al::CodeunitObject &unit, const Objects &objects) {
  const auto absent = [&objects](const al::VarDecl &declared) {
    const std::string type = TypeName(declared.type);
    if (type == "DotNet") { return true; }
    return NamesAnObject(declared) && Reach(declared, objects) == nullptr;
  };
  if (std::ranges::any_of(unit.variables, absent)) { return true; }
  return std::ranges::any_of(unit.procedures, [&absent](const al::ProcedureDecl &procedure) {
    return std::ranges::any_of(procedure.parameters, absent) ||
           std::ranges::any_of(procedure.variables, absent);
  });
}

/// A PAGE IS A TEMPLATE ARGUMENT AND A BASE CLASS, so its header is INCLUDED and never forward
/// declared: `TestPage<pages::PaymentJournal>` derives from the page to reach its controls.
/// A MEMBER OF OBJECT TYPE IS A HANDLE, whatever kind of object it is. It was a codeunit-only rule
/// and the tables disproved that: `Currency Exchange Rate` declares a variable of its own type, so
/// a Record member recurses exactly as a codeunit member does -- and AL cannot be constructing
/// either eagerly, or neither would terminate (board:0037).
bool HandleMember(const al::VarDecl &declared) {
  return NamesAnObject(declared);
}

bool NamesAPage(std::string_view type) {
  return type == "TestPage" || type == "Page" || type == "TestRequestPage";
}

/// Adds the header an index holds for a subtype, when the run translated one.
template <typename Index>
void IndexedHeader(const Index &index, const std::string &subtype, std::set<std::string> &headers) {
  if (subtype.empty()) { return; }
  const auto found = index.find(LowerKey(subtype));
  if (found != index.end() && !found->second.header.empty()) {
    headers.insert(found->second.header);
  }
}

std::string SourceIncludes(const al::CodeunitObject &unit, const Objects &objects) {
  std::set<std::string> headers;
  const auto reach = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared)) { return; }
    const TableRef *ref = Reach(declared, objects);
    if (ref != nullptr && !ref->header.empty()) { headers.insert(ref->header); }
  };
  const auto reachInterface = [&](const al::VarDecl &declared) {
    if (TypeName(declared.type) != "Interface") { return; }
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end() && !found->second.header.empty()) {
      headers.insert(found->second.header);
    }
  };
  const auto named = [&](const al::VarDecl &declared) {
    reach(declared);
    reachInterface(declared);
    if (NamesAPage(TypeName(declared.type))) {
      IndexedHeader(objects.pages, declared.subtype, headers);
    }
  };
  for (const al::VarDecl &declared : unit.variables) { named(declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { named(declared); }
    for (const al::VarDecl &declared : procedure.variables) { named(declared); }
    named(procedure.returned);
  }
  std::string out;
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  return out;
}

/// What a declaration needs the header to KNOW: a name for an object, a name for an interface --
/// which is a pointer -- and the enumeration itself, which is a template argument and not a name.
template <typename Ahead, typename Enum, typename Page, typename Element>
void Declared(const al::VarDecl &declared,
              const Objects &objects,
              Ahead ahead,
              Enum reachEnum,
              Page reachPage,
              Element reachElement) {
  if (NamesAPage(TypeName(declared.type))) { reachPage(declared.subtype); }
  if (NamesAnObject(declared)) {
    const TableRef *ref = Reach(declared, objects);
    // A PLATFORM TABLE ARRIVES WITH THE DOOR and its entry carries no header, so there is nothing
    // to forward declare and nothing to include -- `platform::Field` is `agiru::platform::Field`,
    // not an object in `agiru::app` that this file could declare ahead of itself.
    if (ref != nullptr && !ref->header.empty()) { ahead(ref->identifier); }
  }
  if (TypeName(declared.type) == "Interface") {
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end()) { ahead(found->second.identifier); }
  }
  if (TypeName(declared.type) == "Enum") { reachEnum(declared.subtype); }
  // A GENERIC CARRIES ITS ELEMENT TYPES AND THEY ARE DECLARATIONS TOO. `List of [Enum "Image
  // Analysis Type"]` names an enumeration that nothing else in the file mentions, and the walk
  // stopped at the outer `List`.
  //
  // AND AN ELEMENT IS A MEMBER, not a mention: `Dictionary of [Integer, Codeunit "Temp Blob"]`
  // instantiates `std::pair<Integer, TempBlob>`, which needs the LAYOUT. The handle rule does not
  // reach inside a generic -- what it makes a handle of is the Dictionary, not what the Dictionary
  // holds -- so the element's header is included rather than named. It was the last root of the
  // tree: 9 598 of 9 600 headers compiled and these two did not.
  for (const al::VarDecl &argument : declared.arguments) {
    Declared(argument, objects, ahead, reachEnum, reachPage, reachElement);
    const TableRef *element = ReachObject(argument, objects);
    if (element != nullptr && !element->header.empty()) { reachElement(element->header); }
  }
}

/// Adds the header of an enumeration, when the run has one.
///
/// AN ENUM NEEDS ITS HEADER even where an object needs only its name: `Enum<enums::X>` is a
/// TEMPLATE ARGUMENT and a template argument must be complete. Missing it made `LibraryNoSeries`
/// the first diagnostic of 1 159 failing headers.
void EnumHeader(const EnumIndex &enums,
                const std::string &subtype,
                std::set<std::string> &headers) {
  IndexedHeader(enums, subtype, headers);
}

std::string Includes(const al::CodeunitObject &unit, const Objects &objects) {
  std::set<std::string> headers;
  const auto reach = [&](const al::VarDecl &declared) {
    // A HANDLE NEEDS THE NAME AND NOT THE LAYOUT, so nothing a member holds is included any more.
    if (!NamesAnObject(declared) || HandleMember(declared)) { return; }
    const TableRef *ref = Reach(declared, objects);
    // A PLATFORM TABLE HAS NO HEADER OF ITS OWN TO NAME: it arrives with the door, so its entry
    // carries an empty path and an empty path would emit `#include ""`.
    if (ref != nullptr && !ref->header.empty()) { headers.insert(ref->header); }
  };
  // AN ENUM NEEDS ITS HEADER TOO, and it was the only kind of object this did not ask for:
  // `LibraryNoSeries` names `Enum<enums::NoSeriesImplementation>` and included the table beside it
  // but not the enumeration, which made it the FIRST diagnostic of 1 159 failing headers.
  const auto reachEnum = [&](const std::string &subtype) {
    EnumHeader(objects.enums, subtype, headers);
  };
  // AN INTERFACE NEEDS ITS HEADER TOO, and it is the third kind this walk has had to learn: a
  // variable names it, the abstract class is somewhere else, and nothing else pulls it in.
  std::map<std::string, std::set<std::string>> forward;
  const auto ahead = [&forward](const std::string &qualified) {
    const std::size_t colons = qualified.find("::");
    if (colons == std::string::npos) { return; }
    forward[qualified.substr(0, colons)].insert(qualified.substr(colons + 2));
  };
  // INCLUDE WHAT YOU CONTAIN, DECLARE WHAT YOU NAME -- AND A HEADER CONTAINS ONLY ITS MEMBERS.
  // A function DECLARATION may take and return incomplete types; only a definition or a call needs
  // the layout, and both of those live in the `.cpp`. Including for parameters too made the include
  // graph far larger than the containment graph and produced cycles the containment graph does not
  // have: `Language` holds `LanguageImpl` and `LanguageImpl` holds no codeunit at all, yet their
  // headers included each other because a parameter named the other.
  const auto reachPage = [&](const std::string &subtype) {
    IndexedHeader(objects.pages, subtype, headers);
  };
  const auto reachElement = [&](const std::string &header) { headers.insert(header); };
  const auto both = [&](const al::VarDecl &declared) {
    Declared(declared, objects, ahead, reachEnum, reachPage, reachElement);
  };
  // A MEMBER IS A GLOBAL, so this is the containment graph and nothing else.
  for (const al::VarDecl &declared : unit.variables) { reach(declared); }
  for (const al::VarDecl &declared : unit.variables) { both(declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { both(declared); }
    // AND A PROCEDURE'S OWN VARIABLES, which were never walked at all: a local `Record` or `Enum`
    // is as much a declaration as a parameter is.
    for (const al::VarDecl &declared : procedure.variables) { both(declared); }
    // AND THE RETURN, which is a declaration like any other: `GetAltCustVATRegConsistencyImpl`
    // returns an interface and nothing else in the file names it.
    both(procedure.returned);
  }
  // A CODEUNIT THAT NAMES A .NET TYPE INCLUDES THE GENERATED SURFACE. It is one file for all of
  // them, because the stubs reference nothing but `Refused` and splitting them would be 499 headers
  // for no reader's benefit.
  std::string out = "#include \"agiru.h\"\n";
  // ONLY WHERE SOMETHING IS ACTUALLY ABSENT. An unconditional include would make every generated
  // codeunit depend on a file that exists because something is MISSING, which is the wrong way
  // round -- and it would make the hand-written target image depend on a transpiler run.
  if (NamesAbsent(unit, objects)) { out += "#include \"absent/Types.h\"\n"; }
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }

  // AL LETS TWO CODEUNITS NAME EACH OTHER, AND C++ HEADERS CANNOT. `AOAIFunctionResponse` includes
  // `AOAIChatMessages` and `AOAIChatMessages` includes it back; `#pragma once` makes the second
  // include a no-op, so the inner file sees the outer one half-written and the type is not there.
  // A forward declaration of every object this file NAMES settles it, because a parameter taken by
  // reference needs the name and not the layout -- and AL passes objects by `var` where it passes
  // them at all.
  if (!forward.empty()) { out += "\n"; }
  for (const auto &[space, named] : forward) {
    out += "namespace agiru::app::" + space + " {\n";
    const ObjectKind kind = KindOfNamespace(space);
    for (const std::string &one : named) {
      out += "class " + ClassName(one, kind) + ";\n" + ClassAlias(one, kind);
    }
    out += "} // namespace agiru::app::" + space + "\n";
  }
  out += "\n";
  return out;
}

// THE .NET SURFACE EXISTS ONLY AT THE CALL SITES. A `dotnet` package declares an assembly and a
// type alias and no members at all, so nothing in the AL source says what `XmlDocument` can do --
// only what this corpus ASKS it to do. That set is gathered here, per type, from the tokens of
// every body: a name that is a DotNet variable, a dot, and the member after it (board:0035).
//
// TOKENS AND NOT THE EXPRESSION TREE, because a property read and a call are the same thing here
// and the token pair is the same for both -- `UserInfo.ObjectId` and `Doc.SelectNodes(x)` differ
// only in what follows, which this does not need.
using DotNetNames = std::map<std::string, std::string>;

/// A DECLARATION IS ENOUGH TO OWE THE TYPE, and a call is not required. `O365SyncManagement`
/// takes `Credentials: DotNet ExchangeCredentials` as a parameter and never calls a member on it,
/// so gathering only the CALL SITES left the signature naming a class nobody wrote. The entry is
/// created empty and filled if a member turns up.
void NoteDotNet(const al::VarDecl &declared, DotNetNames &named, DotNetUse &use) {
  if (TypeName(declared.type) == "DotNet" && !declared.subtype.empty()) {
    const std::string bare = Identifier(declared.subtype);
    named.insert_or_assign(LowerKey(declared.name), bare);
    use.try_emplace(bare);
  }
}

/// The field names a record method takes, read straight out of the token stream.
///
/// A FIELD ARGUMENT IS A MEMBER TOO. `X.SetRange("Agent User Security ID", V)` names a field of X,
/// and the emitter spells it against X -- so an absent X needs that name as well, or the stub is
/// missing exactly what the body reaches for. The method decides how many: `SetRange` takes one,
/// `CalcFields` takes all of them (board:0035).
void GatherFieldArguments(const std::vector<al::Token> &tokens,
                          std::size_t method,
                          std::set<std::string> &into) {
  const std::size_t count = FieldArguments(tokens[method].text);
  if (count == 0) { return; }
  std::size_t at = method + 1;
  if (at >= tokens.size() || tokens[at].text != "(") { return; }
  ++at;
  int depth = 0;
  bool first = true;
  for (; at < tokens.size(); ++at) {
    const al::Token &token = tokens[at];
    if (token.text == "(") { ++depth; }
    if (token.text == ")") {
      if (depth == 0) { return; }
      --depth;
    }
    if (depth != 0) { continue; }
    if (token.text == ",") {
      if (count != kEveryArgument) { return; }
      first = true;
      continue;
    }
    if (first && (token.kind == al::TokenKind::Identifier ||
                  token.kind == al::TokenKind::QuotedIdentifier)) {
      into.insert(Identifier(token.text));
    }
    first = false;
  }
}

void GatherCalls(const al::ProcedureDecl &procedure, const DotNetNames &named, DotNetUse &use) {
  for (std::size_t i = 0; i + 2 < procedure.tokens.size(); ++i) {
    if (procedure.tokens[i].kind != al::TokenKind::Identifier &&
        procedure.tokens[i].kind != al::TokenKind::QuotedIdentifier) {
      continue;
    }
    if (procedure.tokens[i + 1].text != ".") { continue; }
    // A QUOTED NAME IS A NAME. AL writes `Agent."Display Name" := X` and the gather only accepted
    // the bare kind, so every member whose AL name has a space in it was missing from the stub --
    // which is most of them, because BC names fields the way a caption reads.
    if (procedure.tokens[i + 2].kind != al::TokenKind::Identifier &&
        procedure.tokens[i + 2].kind != al::TokenKind::QuotedIdentifier) {
      continue;
    }
    const auto found = named.find(LowerKey(procedure.tokens[i].text));
    if (found == named.end()) { continue; }
    use[found->second].insert(Identifier(procedure.tokens[i + 2].text));
    // A FIELD ARGUMENT IS A MEMBER TOO. `X.SetRange("Agent User Security ID", V)` names a field of
    // X, and the emitter spells it against X -- so an absent X needs that name as well, or the
    // stub is missing exactly what the body reaches for. The method decides: `SetRange` takes one
    // field, `CalcFields` takes all of them (board:0035).
    GatherFieldArguments(procedure.tokens, i + 2, use[found->second]);
  }
}

// AN AL OBJECT THIS RUN DOES NOT HAVE IS THE SAME QUESTION AS A .NET TYPE. `Record "Windows
// Language"` names a platform table no source root declares, so the generator emitted the bare
// identifier and the file stopped. What the corpus asks of it is in the call sites, exactly as it
// is for `DotNet` -- so it is gathered by the same walk and answered by the same shape.
// A TYPE THAT IS NAMED BUT NEVER ASKED ANYTHING STILL NEEDS TO EXIST. `Verbosity` appears as a
// parameter type and nothing is ever called on it, so gathering only members would leave the
// declaration pointing at nothing. The entry is created empty and filled if a member turns up.
void NoteAbsent(const al::VarDecl &declared,
                const Objects &objects,
                DotNetNames &named,
                DotNetUse &use) {
  if (NamesAbsentType(declared)) {
    const std::string bare = Identifier(declared.type);
    named.insert_or_assign(LowerKey(declared.name), bare);
    use.try_emplace(bare);
    return;
  }
  if (!NamesAnObject(declared) || Reach(declared, objects) != nullptr) { return; }
  const std::string subtype = Identifier(declared.subtype);
  named.insert_or_assign(LowerKey(declared.name), subtype);
  use.try_emplace(subtype);
}

void GatherDotNet(const al::CodeunitObject &unit,
                  const Objects &objects,
                  DotNetUse &use,
                  DotNetUse &absent) {
  DotNetNames named;
  DotNetNames missing;
  for (const al::VarDecl &declared : unit.variables) {
    NoteDotNet(declared, named, use);
    NoteAbsent(declared, objects, missing, absent);
  }

  for (const al::ProcedureDecl &procedure : unit.procedures) {
    DotNetNames inner = named;
    DotNetNames innerMissing = missing;
    for (const al::VarDecl &declared : procedure.parameters) {
      NoteDotNet(declared, inner, use);
      NoteAbsent(declared, objects, innerMissing, absent);
    }
    for (const al::VarDecl &declared : procedure.variables) {
      NoteDotNet(declared, inner, use);
      NoteAbsent(declared, objects, innerMissing, absent);
    }
    GatherCalls(procedure, inner, use);
    GatherCalls(procedure, innerMissing, absent);
  }
}

std::vector<std::string> Unresolved(const al::CodeunitObject &unit, const Objects &objects) {
  std::vector<std::string> missing;
  const auto note = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared) || Reach(declared, objects) != nullptr) { return; }
    if (std::ranges::find(missing, declared.subtype) == missing.end()) {
      missing.push_back(declared.subtype);
    }
  };
  for (const al::VarDecl &declared : unit.variables) { note(declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { note(declared); }
  }
  return missing;
}

// A PROCEDURE'S SCOPE, INNERMOST FIRST: its own locals, then its parameters, then the codeunit's
// variables, labels and other procedures. AL resolves a bare name that way and so does C++ once the
// locals are declared, so every one of them spells itself -- what this settles is that the name IS
// known, which is what keeps an unknown one from being emitted as if it were.
class CodeunitNames : public Names {
public:
  CodeunitNames(const al::CodeunitObject &unit,
                const al::ProcedureDecl &procedure,
                const Objects &objects)
      : unit_(unit), procedure_(procedure), objects_(objects) {}

  /// The record variable's table, then the field, then what the field was declared as.
  [[nodiscard]] bool IsRecord(std::string_view variable) const override {
    const al::VarDecl *declared = Declaration(variable);
    return declared != nullptr && TypeName(declared->type) == "Record";
  }

  /// A door type has no fields, so a member of one is a call however AL spelled it.
  [[nodiscard]] bool MembersAreCalls(std::string_view variable) const override {
    const al::VarDecl *declared = Declaration(variable);
    if (declared == nullptr) { return false; }
    const std::string type = TypeName(declared->type);
    // Every door type whose members are all methods. A `Variant` is one: AL asks it `IsOption`
    // and `IsRecord` without parentheses, and it has no fields to confuse them with.
    return type == "RecordRef" || type == "FieldRef" || type == "KeyRef" || type == "Variant" ||
           type == "RecordId" || type == "ModuleInfo" || type == "Version";
  }

  [[nodiscard]] std::string FieldEnumeration(const OfVariable &field) const override {
    const al::VarDecl *declared = Declaration(field.variable);
    if (declared == nullptr || TypeName(declared->type) != "Record") { return {}; }
    const auto table = objects_.fieldEnums.find(LowerKey(declared->subtype));
    if (table == objects_.fieldEnums.end()) { return {}; }
    const auto found = table->second.find(LowerKey(std::string(field.field)));
    return found == table->second.end() ? std::string{} : found->second;
  }

  [[nodiscard]] const al::VarDecl *Declaration(std::string_view name) const {
    const auto same = [&name](const al::VarDecl &declared) {
      return LowerKey(declared.name) == LowerKey(std::string(name));
    };
    for (const al::VarDecl &declared : procedure_.variables) {
      if (same(declared)) { return &declared; }
    }
    for (const al::VarDecl &declared : procedure_.parameters) {
      if (same(declared)) { return &declared; }
    }
    for (const al::VarDecl &declared : unit_.variables) {
      if (same(declared)) { return &declared; }
    }
    return nullptr;
  }

  [[nodiscard]] bool IsHandle(std::string_view name) const override {
    const auto same = [&name](const al::VarDecl &declared) {
      return LowerKey(declared.name) == LowerKey(std::string(name));
    };
    // A LOCAL WINS OVER A MEMBER, the same order `Resolve` uses, and a local is a value.
    if (std::ranges::any_of(procedure_.variables, same)) { return false; }
    if (std::ranges::any_of(procedure_.parameters, same)) { return false; }
    for (const al::VarDecl &declared : unit_.variables) {
      if (same(declared)) { return NamesAnObject(declared); }
    }
    return false;
  }

  /// \note THE DECLARATION'S SPELLING WINS AND NOT THE CALL SITE'S. AL is case-insensitive, so
  ///       `AgentConsumptionOverview` declares `AgentUserSecurityId` and its body writes
  ///       `AgentUserSecurityID`; returning what the CALLER wrote made those two different C++
  ///       symbols and the second one named nothing. CLAUDE.md lists it as a measured failure mode
  ///       -- "collapse match, once, in the generator" -- and this is the one place it happens.
  [[nodiscard]] std::string Resolve(std::string_view name) const override {
    for (const al::VarDecl &declared : procedure_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return Identifier(declared.name);
      }
    }
    for (const al::VarDecl &declared : procedure_.parameters) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return Identifier(declared.name);
      }
    }
    if (!procedure_.returnName.empty() &&
        LowerKey(procedure_.returnName) == LowerKey(std::string(name))) {
      return Identifier(procedure_.returnName);
    }
    for (const al::VarDecl &declared : unit_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return Identifier(declared.name);
      }
    }
    for (const al::LabelDecl &label : unit_.labels) {
      if (LowerKey(label.name) == LowerKey(std::string(name))) { return Identifier(label.name); }
    }
    for (const al::ProcedureDecl &other : unit_.procedures) {
      if (LowerKey(other.name) == LowerKey(std::string(name))) { return Identifier(other.name); }
    }
    return {};
  }

  /// \note EMPTY UNTIL A VARIABLE'S ENUM CAN BE NAMED. `X::Member` where `X` is an option or enum
  ///       variable needs the enumeration that declared it, which is an index this writer does not
  ///       carry yet. Returning nothing emits `X::Member` against a variable, which does not
  ///       compile -- loud, and at build time, rather than a plausible wrong type.
  [[nodiscard]] std::string Enumeration(std::string_view name) const override {
    static_cast<void>(name);
    return {};
  }

private:
  const al::CodeunitObject &unit_;
  const al::ProcedureDecl &procedure_;
  const Objects &objects_;
};

std::string Locals(const al::ProcedureDecl &procedure,
                   const Objects &objects,
                   const std::string &unit,
                   const std::vector<al::ProcedureDecl> &all = {}) {
  std::string out;
  // THE NAMED RETURN VALUE IS A LOCAL, and it comes first because AL declares it in the signature,
  // ahead of the var block. `exit;` with no argument returns it, zero-initialised if nothing wrote.
  if (!procedure.returnName.empty()) {
    out += "  " + Returns(procedure, objects) + " " + Identifier(procedure.returnName) + "{};\n";
  }
  // A LOCAL NAMED AFTER ITS TYPE HIDES IT FOR EVERY LOCAL AFTER IT. AL writes
  // `FieldRef: FieldRef;` and then declares another of the same type; the second one names the
  // variable. It is the same rule the parameters follow, over the same scope.
  std::set<std::string> names;
  for (const al::VarDecl &declared : procedure.variables) {
    names.insert(Identifier(declared.name));
  }
  if (!procedure.returnName.empty()) { names.insert(Identifier(procedure.returnName)); }
  for (const al::VarDecl &declared : procedure.variables) {
    std::string type = TypeOf(declared, objects, OptionNameOf(unit, procedure.name, declared, all));
    if (Hidden(type, names)) { type = Qualified(type, names); }
    out += "  " + type + " " + Identifier(declared.name) + "{};\n";
  }
  return out;
}

} // namespace

std::string WriteCodeunitSource(const al::CodeunitObject &unit,
                                const std::string &sourcePath,
                                const Objects &objects) {
  const std::string identifier = Identifier(unit.name);
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#include \"" + identifier + ".h\"\n\n";
  out += "#include \"agiru.h\"\n";
  // THE SOURCE INCLUDES WHAT THE HEADER ONLY DECLARED. The header carries the containment graph and
  // forward-declares everything else, which is what keeps its include graph acyclic; a BODY calls
  // methods on those objects and needs their layout, so the definitions bring them in. Header
  // declares, source includes -- and the cycle the header cannot have, the source can, because a
  // `.cpp` is nobody's dependency.
  out += SourceIncludes(unit, objects);
  out += "\nnamespace agiru::app::codeunits {\n\n";

  for (const al::ProcedureDecl &procedure : unit.procedures) {
    // AN EVENT PUBLISHER'S BODY IS EMPTY BY DESIGN -- the platform fires subscribers at the CALL
    // SITE -- so its definition has nothing to do with its parameters and drops their names.
    const bool publisher = IsPublisher(procedure);
    out += Returns(procedure, objects) + " " + identifier + "::" + Identifier(procedure.name) +
           "(" + Parameters(procedure, objects, !publisher, unit.name, {}, unit.procedures) + ") {";
    const std::string locals =
        publisher ? std::string{} : Locals(procedure, objects, unit.name, unit.procedures);
    const std::string body =
        publisher ? std::string{}
                  : WriteStatements(CodeunitNames(unit, procedure, objects), procedure.body, 2);
    if (locals.empty() && body.empty()) {
      out += "}\n\n";
      continue;
    }
    out += "\n";
    out += locals;
    if (!locals.empty() && !body.empty()) { out += "\n"; }
    out += body;
    out += "}\n\n";
  }

  out += "} // namespace agiru::app::codeunits\n";
  return out;
}

std::string ProcedureSignature(const al::ProcedureDecl &procedure,
                               const Objects &objects,
                               const std::string &owner,
                               const std::string &qualifier,
                               bool named,
                               const std::set<std::string> &shadowed,
                               const std::vector<al::ProcedureDecl> &all,
                               const std::string &spelled) {
  std::set<std::string> hiding = shadowed;
  hiding.insert(Identifier(procedure.name));
  const std::string name = spelled.empty() ? Identifier(procedure.name) : spelled;
  return Returns(procedure, objects, hiding) + " " + qualifier + "::" + name + "(" +
         Parameters(procedure, objects, named, owner, hiding, all) + ")";
}

std::string ProcedureLocals(const al::ProcedureDecl &procedure,
                            const Objects &objects,
                            const std::string &owner,
                            const std::vector<al::ProcedureDecl> &all) {
  return Locals(procedure, objects, owner, all);
}

std::string SourceIncludesOf(const std::vector<al::VarDecl> &variables,
                             const std::vector<al::ProcedureDecl> &procedures,
                             const Objects &objects) {
  al::CodeunitObject unit;
  unit.variables = variables;
  unit.procedures = procedures;
  return SourceIncludes(unit, objects);
}

std::string ProcedureDeclaration(const al::ProcedureDecl &procedure,
                                 const Objects &objects,
                                 const std::string &owner,
                                 const std::set<std::string> &shadowed,
                                 const std::vector<al::ProcedureDecl> &all,
                                 const std::string &spelled) {
  std::string line = Declaration(procedure, objects, owner, shadowed, all);
  if (spelled.empty() || spelled == Identifier(procedure.name)) { return line; }
  const std::string wanted = " " + Identifier(procedure.name) + "(";
  const std::size_t at = line.find(wanted);
  if (at == std::string::npos) { return line; }
  return line.substr(0, at) + " " + spelled + "(" + line.substr(at + wanted.size());
}

std::string QualifiedType(const std::string &type, const std::set<std::string> &names) {
  return Hidden(type, names) ? Qualified(type, names) : type;
}

std::string DeclaredType(const al::VarDecl &declared, const Objects &objects) {
  return TypeOf(declared, objects);
}

void GatherAbsentIn(const std::vector<al::VarDecl> &variables,
                    const std::vector<al::ProcedureDecl> &procedures,
                    const Objects &objects,
                    DotNetUse &dotnet,
                    DotNetUse &absent) {
  al::CodeunitObject unit;
  unit.variables = variables;
  unit.procedures = procedures;
  GatherDotNet(unit, objects, dotnet, absent);
}

bool NamesAbsentIn(const std::vector<al::VarDecl> &variables,
                   const std::vector<al::ProcedureDecl> &procedures,
                   const Objects &objects) {
  al::CodeunitObject unit;
  unit.variables = variables;
  unit.procedures = procedures;
  return NamesAbsent(unit, objects);
}

bool DeclaresAnObject(const al::VarDecl &declared) {
  return NamesAnObject(declared);
}

const TableRef *ReachObject(const al::VarDecl &declared, const Objects &objects) {
  return NamesAnObject(declared) ? Reach(declared, objects) : nullptr;
}

std::string InlineOptionsOf(const std::string &owner,
                            const std::string &space,
                            const std::vector<al::VarDecl> &variables,
                            const std::vector<al::ProcedureDecl> &procedures) {
  return InlineOptionsIn(owner, space, variables, procedures);
}

std::string CodeunitHeaderPath(const al::CodeunitObject &unit) {
  return OutputDirectory(unit.nameSpace, ObjectKind::Codeunit) + "/" + Identifier(unit.name) + ".h";
}

TableIndex PlatformTables() {
  TableIndex tables;
  const auto add = [&tables](std::string_view name, std::string_view number) {
    const TableRef ref{.identifier = "platform::" + Identifier(name), .header = {}};
    tables.insert_or_assign(LowerKey(std::string(name)), ref);
    tables.insert_or_assign(std::string(number), ref);
  };
  add("Field", "2000000041");
  add("Integer", "2000000026");
  add("Date", "2000000007");
  add("User", "2000000120");
  return tables;
}

/// The headers and the forward declarations an interface needs. A SIGNATURE IS A DECLARATION, so
/// what it names is reached the way a codeunit reaches its own -- and an interface variable is a
/// pointer, so another interface is only named (board:0027).
namespace {

void FaceReach(const al::VarDecl &declared,
               const Objects &objects,
               std::set<std::string> &headers,
               std::map<std::string, std::set<std::string>> &forward) {
  for (const al::VarDecl &argument : declared.arguments) {
    FaceReach(argument, objects, headers, forward);
  }
  // AN INTERFACE NAMES ANOTHER INTERFACE, and that one is a POINTER: `Price Calculation` takes a
  // `Line With Price`. A declaration needs the name, so it is forward declared and the two files do
  // not include each other.
  if (TypeName(declared.type) == "Interface") {
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end()) {
      forward["interfaces"].insert(found->second.identifier.substr(std::size("interfaces::") - 1));
    }
    return;
  }
  if (NamesAnObject(declared)) {
    const TableRef *ref = Reach(declared, objects);
    if (ref != nullptr && !ref->header.empty()) { headers.insert(ref->header); }
  }
  if (TypeName(declared.type) == "Enum" && !declared.subtype.empty()) {
    const auto found = objects.enums.find(LowerKey(declared.subtype));
    if (found != objects.enums.end() && !found->second.header.empty()) {
      headers.insert(found->second.header);
    }
  }
}

std::string FaceDeclarations(const al::InterfaceObject &object, const Objects &objects) {
  std::string out;
  // AN INTERFACE INCLUDES WHAT ITS SIGNATURES NAME, exactly as a codeunit includes what its
  // declarations name -- a signature is a declaration and a `Verbosity` in one is an enum the file
  // has to have seen.
  std::set<std::string> headers;
  std::map<std::string, std::set<std::string>> forward;
  for (const al::ProcedureDecl &procedure : object.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) {
      FaceReach(declared, objects, headers, forward);
    }
    FaceReach(procedure.returned, objects, headers, forward);
  }
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  if (NamesAbsentIn({}, object.procedures, objects)) { out += "#include \"absent/Types.h\"\n"; }
  for (const auto &[space, named] : forward) {
    out += "\nnamespace agiru::app::" + space + " {\n";
    for (const std::string &one : named) {
      out += "class " + ClassName(one, ObjectKind::Interface) + ";\n" +
             ClassAlias(one, ObjectKind::Interface);
    }
    out += "} // namespace agiru::app::" + space + "\n";
  }
  return out;
}

} // namespace

InterfaceHeader WriteInterface(const al::InterfaceObject &object,
                               const std::string &sourcePath,
                               const Objects &objects) {
  const std::string identifier = Identifier(object.name);
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n#pragma once\n\n#include \"agiru.h\"\n";
  out += FaceDeclarations(object, objects);
  out += "\nnamespace agiru::app::interfaces {\n\n";
  const std::string faceClass = ClassName(identifier, ObjectKind::Interface);
  out += "class " + faceClass + ";\n" + ClassAlias(identifier, ObjectKind::Interface) + "\n";
  out += "class " + faceClass + " {\n";
  out += "public:\n";
  // A CLASS SOMEBODY DERIVES FROM NEEDS A VIRTUAL DESTRUCTOR, and an interface is only ever
  // derived from.
  out += "  virtual ~" + faceClass + "() = default;\n\n";
  for (const al::ProcedureDecl &procedure : object.procedures) {
    out += "  virtual " + Returns(procedure, objects) + " " + Identifier(procedure.name) + "(" +
           Parameters(procedure, objects, true, object.name) + ") = 0;\n";
  }
  out += "};\n\n} // namespace agiru::app::interfaces\n";
  // AN INTERFACE'S SIGNATURES NAME .NET TYPES TOO, and gathering only the absent ones left the
  // dotnet surface short of what the declarations need.
  DotNetUse missing;
  DotNetUse dotnet;
  GatherAbsentIn({}, object.procedures, objects, dotnet, missing);
  return InterfaceHeader{.text = out, .absent = std::move(missing), .dotnet = std::move(dotnet)};
}

CodeunitHeader WriteCodeunit(const al::CodeunitObject &unit,
                             const std::string &sourcePath,
                             const Objects &objects) {
  const std::string identifier = Identifier(unit.name);
  const std::set<std::string> shadowed = Shadowing(unit.variables, unit.procedures, unit.labels);

  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#pragma once\n\n";
  out += Includes(unit, objects);
  out += "#include <array>\n#include <cstdint>\n#include <string_view>\n\n";
  out += InlineOptions(unit);

  out += "namespace agiru::app::codeunits {\n\n";
  const std::string unitClass = ClassName(identifier, ObjectKind::Codeunit);
  out += "class " + unitClass + ";\n" + ClassAlias(identifier, ObjectKind::Codeunit) + "\n";
  out += "class " + unitClass + " : public Codeunit<" + unitClass + "> {\npublic:\n";

  // PUBLIC BEFORE PRIVATE, AND `local` IS WHAT DECIDES IT. AL's `local procedure` is exactly C++'s
  // private, and an event publisher is local too -- nobody calls it but the object that raises it.
  //
  // THE ORDER IS AL'S ORDER, with a blank line where the KIND changes. A `trigger` is called by the
  // platform and a `procedure` by whoever holds the object; AL says which, and a reader scanning
  // the class wants that boundary. Reordering to group them would read less like the .al than the
  // blank line costs.
  bool previousWasTrigger = false;
  bool first = true;
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    if (procedure.isLocal) { continue; }
    if (!first && previousWasTrigger != procedure.isTrigger) { out += "\n"; }
    out += Declaration(procedure, objects, unit.name, shadowed, unit.procedures);
    previousWasTrigger = procedure.isTrigger;
    first = false;
  }

  std::string hidden;
  for (const al::VarDecl &declared : unit.variables) {
    std::string type = TypeOf(declared, objects, OptionName(unit.name, {}, declared.name));
    if (Hidden(type, shadowed)) { type = Qualified(type, shadowed); }
    const bool handle = HandleMember(declared);
    hidden +=
        "  " + (handle ? "Instance<" + type + ">" : type) + " " + Identifier(declared.name) + ";\n";
  }
  if (!unit.labels.empty() && !hidden.empty()) { hidden += "\n"; }
  for (const al::LabelDecl &label : unit.labels) {
    hidden += "  static constexpr std::string_view " + Identifier(label.name) + "{" +
              Literal(label.text) + "};\n";
  }
  std::string locals;
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    if (!procedure.isLocal) { continue; }
    locals += Declaration(procedure, objects, unit.name, shadowed, unit.procedures);
  }
  if (!hidden.empty() || !locals.empty()) {
    out += "\nprivate:\n";
    out += hidden;
    if (!hidden.empty() && !locals.empty()) { out += "\n"; }
    out += locals;
  }
  out += "};\n\n";
  out += "} // namespace agiru::app::codeunits\n\n";

  out += "template <> struct agiru::CodeunitTraits<agiru::app::codeunits::" + identifier + "> {\n";
  out += "  static constexpr CodeunitId kId{" + std::to_string(unit.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(unit.name) + "};\n";
  out += "};\n";
  DotNetUse dotnet;
  DotNetUse absent;
  GatherDotNet(unit, objects, dotnet, absent);
  return CodeunitHeader{.text = out,
                        .unresolvedTables = Unresolved(unit, objects),
                        .dotnet = std::move(dotnet),
                        .absent = std::move(absent)};
}

} // namespace agiru::gen
