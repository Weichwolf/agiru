#include "Door.h"

#include "Scope.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <set>
#include <string>
#include <string_view>
#include <utility>

namespace agiru::gen {

namespace {

// ONE DOOR HEADER PER AL TYPE, and the file stem IS the type name -- which is what makes this table
// derivable rather than invented. `make lint` holds it against the directory, so a type the door
// gains and this table does not is a finding rather than a compile error in 5 835 files.
constexpr std::array kTypes{
    std::string_view{"Action"},
    std::string_view{"AlArray"},
    std::string_view{"Any"},
    std::string_view{"AuditCategory"},
    std::string_view{"BigInteger"},
    std::string_view{"BigText"},
    std::string_view{"Blob"},
    std::string_view{"Boolean"},
    std::string_view{"Byte"},
    std::string_view{"Char"},
    std::string_view{"ClientType"},
    std::string_view{"Code"},
    std::string_view{"CommitBehavior"},
    std::string_view{"CompanyProperty"},
    std::string_view{"Cookie"},
    std::string_view{"DataClassification"},
    std::string_view{"DataScope"},
    std::string_view{"DataTransfer"},
    std::string_view{"Date"},
    std::string_view{"DateFormula"},
    std::string_view{"DateTime"},
    std::string_view{"Debugger"},
    std::string_view{"Decimal"},
    std::string_view{"DefaultLayout"},
    std::string_view{"Dialog"},
    std::string_view{"Dictionary"},
    std::string_view{"Duration"},
    std::string_view{"Enum"},
    std::string_view{"ErrorBehavior"},
    std::string_view{"ErrorInfo"},
    std::string_view{"ErrorType"},
    std::string_view{"ExecutionContext"},
    std::string_view{"ExecutionMode"},
    std::string_view{"FieldClass"},
    std::string_view{"File"},
    std::string_view{"FileUpload"},
    std::string_view{"FilterPageBuilder"},
    std::string_view{"Guid"},
    std::string_view{"HttpClient"},
    std::string_view{"HttpContent"},
    std::string_view{"HttpHeaders"},
    std::string_view{"HttpRequestMessage"},
    std::string_view{"HttpRequestType"},
    std::string_view{"HttpResponseMessage"},
    std::string_view{"InherentPermissionsScope"},
    std::string_view{"Integer"},
    std::string_view{"IsolatedStorage"},
    std::string_view{"IsolationLevel"},
    std::string_view{"JsonArray"},
    std::string_view{"JsonObject"},
    std::string_view{"JsonToken"},
    std::string_view{"JsonValue"},
    std::string_view{"KeyRef"},
    std::string_view{"Label"},
    std::string_view{"List"},
    std::string_view{"Media"},
    std::string_view{"MediaSet"},
    std::string_view{"ModuleInfo"},
    std::string_view{"NavApp"},
    std::string_view{"Notification"},
    std::string_view{"NotificationScope"},
    std::string_view{"NumberSequence"},
    std::string_view{"ObjectType"},
    std::string_view{"Option"},
    std::string_view{"PageBackgroundTaskErrorLevel"},
    std::string_view{"PageStyle"},
    std::string_view{"PermissionObjectType"},
    std::string_view{"ProductName"},
    std::string_view{"PromptMode"},
    std::string_view{"RecordId"},
    std::string_view{"ReportFormat"},
    std::string_view{"ReportLayoutType"},
    std::string_view{"SecretText"},
    std::string_view{"SecurityFilter"},
    std::string_view{"SecurityOperationResult"},
    std::string_view{"SessionInformation"},
    std::string_view{"SessionSettings"},
    std::string_view{"Stream"},
    std::string_view{"StringValue"},
    std::string_view{"TableConnectionType"},
    std::string_view{"TableFilter"},
    std::string_view{"TaskScheduler"},
    std::string_view{"TelemetryScope"},
    std::string_view{"TestHttpRequestMessage"},
    std::string_view{"TestHttpResponseMessage"},
    std::string_view{"TestPage"},
    std::string_view{"TestPermissions"},
    std::string_view{"Text"},
    std::string_view{"TextBuilder"},
    std::string_view{"TextConst"},
    std::string_view{"TextEncoding"},
    std::string_view{"Time"},
    std::string_view{"TransactionModel"},
    std::string_view{"TransactionType"},
    std::string_view{"Variant"},
    std::string_view{"Verbosity"},
    std::string_view{"Version"},
    std::string_view{"WebServiceActionContext"},
    std::string_view{"WebServiceActionResultCode"},
    std::string_view{"XmlAttribute"},
    std::string_view{"XmlAttributeCollection"},
    std::string_view{"XmlCData"},
    std::string_view{"XmlComment"},
    std::string_view{"XmlDeclaration"},
    std::string_view{"XmlDocument"},
    std::string_view{"XmlDocumentType"},
    std::string_view{"XmlElement"},
    std::string_view{"XmlNameTable"},
    std::string_view{"XmlNamespaceManager"},
    std::string_view{"XmlNode"},
    std::string_view{"XmlNodeList"},
    std::string_view{"XmlProcessingInstruction"},
    std::string_view{"XmlReadOptions"},
    std::string_view{"XmlText"},
    std::string_view{"XmlWriteOptions"},
};

// The names that are not a type's own header: a runtime class, a platform table, a .NET class or
// the metadata a generated table declares.
constexpr std::array<std::pair<std::string_view, std::string_view>, 27> kElsewhere{{
    {"Temporary", "runtime/Table.h"},
    {"TestPage", "type/TestPage.h"},
    {"TempStore", "runtime/Table.h"},
    {"Instance", "runtime/Codeunit.h"},
    {"CodeunitTraits", "runtime/Codeunit.h"},
    {"TableTraits", "runtime/Table.h"},
    {"PageTraits", "runtime/Page.h"},
    {"Commit", "runtime/Transaction.h"},
    {"RecordRef", "runtime/RecordRef.h"},
    {"FieldRef", "runtime/RecordRef.h"},
    {"TestCatalogue", "runtime/TestRunner.h"},
    {"InvokeTest", "runtime/TestRunner.h"},
    {"TestMethod", "runtime/TestRunner.h"},
    {"RegisterTable", "runtime/Catalogue.h"},
    {"SelectLatestVersion", "runtime/Database.h"},
    {"GetLastErrorText", "runtime/Error.h"},
    {"AssertError", "runtime/Error.h"},
    {"platform::", "platform/Field.h"},
    {"absent::", "dotnet/Refused.h"},
    {"DotNetGeneric", "dotnet/Generic.h"},
    {"ALConfigSettings", "dotnet/ALConfigSettings.h"},
    {"NavTenantSettingsHelper", "dotnet/NavTenantSettingsHelper.h"},
    {"UserInfo", "dotnet/UserInfo.h"},
    {"StrSubstNo", "runtime/Record.h"},
    {"Format", "runtime/Record.h"},
    {"AsText", "runtime/Record.h"},
    {"FieldNo", "meta/Ids.h"},
}};

bool Mentions(std::string_view text, std::string_view name) {
  for (std::size_t at = text.find(name); at != std::string_view::npos;
       at = text.find(name, at + 1)) {
    const bool before = at > 0 && (std::isalnum(static_cast<unsigned char>(text[at - 1])) != 0 ||
                                   text[at - 1] == '_');
    const std::size_t after = at + name.size();
    const bool behind =
        after < text.size() &&
        (std::isalnum(static_cast<unsigned char>(text[after])) != 0 || text[after] == '_');
    if (!before && (!behind || name.ends_with("::"))) { return true; }
  }
  return false;
}

} // namespace

std::string DoorIncludes(std::string_view text, ObjectKind kind) {
  std::set<std::string> headers;
  // EVERY BODY CAN RAISE AND EVERY OBJECT CARRIES ITS NUMBER, so those two are unconditional.
  headers.insert("meta/Ids.h");
  headers.insert("runtime/Error.h");
  switch (kind) {
    case ObjectKind::Table:
      headers.insert("meta/Declare.h");
      headers.insert("meta/TableDef.h");
      headers.insert("runtime/Table.h");
      break;
    case ObjectKind::Codeunit: headers.insert("runtime/Codeunit.h"); break;
    case ObjectKind::Page: headers.insert("runtime/Page.h"); break;
    case ObjectKind::Enum: headers.insert("meta/EnumDef.h"); break;
    default: break;
  }
  for (const std::string_view type : kTypes) {
    if (Mentions(text, type)) { headers.insert("type/" + std::string(type) + ".h"); }
  }
  for (const auto &[name, header] : kElsewhere) {
    if (Mentions(text, name)) { headers.insert(std::string(header)); }
  }
  // A BUILTIN IS A BARE NAME AND THERE ARE 154 OF THEM, so the file that calls one is not found by
  // asking after each; `Builtins.h` comes in whenever a body was written at all.
  if (text.find(") {\n") != std::string_view::npos) { headers.insert("Builtins.h"); }
  std::string out;
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  return out;
}

std::string WithDoor(std::string text, ObjectKind kind) {
  const std::size_t at = text.find(kDoorMarker);
  if (at == std::string::npos) { return text; }
  std::string without = text;
  without.erase(at, kDoorMarker.size());
  // A BLANK LINE AFTER THE DOOR, because what follows it is the OBJECTS this file names -- two
  // different kinds of dependency, and AL separates them too: `namespace` then `using`.
  return without.substr(0, at) + DoorIncludes(without, kind) + "\n" + without.substr(at);
}

} // namespace agiru::gen
