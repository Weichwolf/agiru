#pragma once

/// \file
/// \brief The one header a generated file includes, and the map of what stands behind it.
///
/// FIVE DIRECTORIES, AND EACH ANSWERS A DIFFERENT QUESTION. A new header goes where its question
/// belongs, and the rule is stated here rather than inferred from what happens to be next to it:
///
/// | directory | what it holds |
/// |---|---|
/// | `type/` | AL's VALUE types -- `Integer`, `Text`, `Date`, `Variant`, `List`. One header per AL
/// type, named as AL names it | | `meta/` | the `constexpr` metadata a generated object carries:
/// `TableDef`, `FieldDef`, `Declare`, the strong ids | | `runtime/` | the MACHINERY: `Table`,
/// `Record`, `Session`, `Transaction`, `Storage`, `RecordRef`. What AL never names and always uses
/// | | `platform/` | AL OBJECTS the platform provides rather than an app -- the virtual and system
/// tables (board:0032). One of eight so far | | `dotnet/` | the .NET classes AL names, and
/// `Refused` for the members not rebuilt (board:0035) |
///
/// The line between `runtime/` and `platform/` is the one worth stating: `runtime/` is what agiru
/// IS, `platform/` is what BC PROVIDES. `TenantSettings` moved across it -- a tenant's deployment
/// facts are something the platform states, not machinery agiru runs on.

#include "Builtins.h"
#include "dotnet/ALConfigSettings.h"
#include "dotnet/Generic.h"
#include "dotnet/NavTenantSettingsHelper.h"
#include "dotnet/Refused.h"
#include "dotnet/UserInfo.h"
#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Date.h"
#include "platform/Field.h"
#include "platform/Integer.h"
#include "platform/Tenant.h"
#include "platform/User.h"
#include "runtime/Catalogue.h"
#include "runtime/Codeunit.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Indexing.h"
#include "runtime/Page.h"
#include "runtime/Record.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TestRunner.h"
#include "type/Action.h"
#include "type/AlArray.h"
#include "type/Any.h"
#include "type/AuditCategory.h"
#include "type/BigInteger.h"
#include "type/BigText.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/ClientType.h"
#include "type/Code.h"
#include "type/CommitBehavior.h"
#include "type/CompanyProperty.h"
#include "type/Cookie.h"
#include "type/DataClassification.h"
#include "type/DataScope.h"
#include "type/DataTransfer.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Debugger.h"
#include "type/Decimal.h"
#include "type/DefaultLayout.h"
#include "type/Dialog.h"
#include "type/Dictionary.h"
#include "type/Duration.h"
#include "type/Enum.h"
#include "type/ErrorBehavior.h"
#include "type/ErrorInfo.h"
#include "type/ErrorType.h"
#include "type/ExecutionContext.h"
#include "type/ExecutionMode.h"
#include "type/FieldClass.h"
#include "type/File.h"
#include "type/FileUpload.h"
#include "type/FilterPageBuilder.h"
#include "type/Guid.h"
#include "type/HttpClient.h"
#include "type/HttpContent.h"
#include "type/HttpHeaders.h"
#include "type/HttpRequestMessage.h"
#include "type/HttpRequestType.h"
#include "type/HttpResponseMessage.h"
#include "type/InherentPermissionsScope.h"
#include "type/Integer.h"
#include "type/IsolatedStorage.h"
#include "type/IsolationLevel.h"
#include "type/JsonArray.h"
#include "type/JsonObject.h"
#include "type/JsonToken.h"
#include "type/JsonValue.h"
#include "type/KeyRef.h"
#include "type/Label.h"
#include "type/List.h"
#include "type/Media.h"
#include "type/MediaSet.h"
#include "type/ModuleInfo.h"
#include "type/NavApp.h"
#include "type/Notification.h"
#include "type/NotificationScope.h"
#include "type/NumberSequence.h"
#include "type/ObjectType.h"
#include "type/Option.h"
#include "type/PageBackgroundTaskErrorLevel.h"
#include "type/PageStyle.h"
#include "type/PermissionObjectType.h"
#include "type/ProductName.h"
#include "type/PromptMode.h"
#include "type/RecordId.h"
#include "type/ReportFormat.h"
#include "type/ReportLayoutType.h"
#include "type/SecretText.h"
#include "type/SecurityFilter.h"
#include "type/SecurityOperationResult.h"
#include "type/SessionInformation.h"
#include "type/SessionSettings.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/TableConnectionType.h"
#include "type/TableFilter.h"
#include "type/TaskScheduler.h"
#include "type/TelemetryScope.h"
#include "type/TestHttpRequestMessage.h"
#include "type/TestHttpResponseMessage.h"
#include "type/TestPage.h"
#include "type/TestPermissions.h"
#include "type/Text.h"
#include "type/TextBuilder.h"
#include "type/TextConst.h"
#include "type/TextEncoding.h"
#include "type/Time.h"
#include "type/TransactionModel.h"
#include "type/TransactionType.h"
#include "type/Variant.h"
#include "type/Verbosity.h"
#include "type/Version.h"
#include "type/WebServiceActionContext.h"
#include "type/WebServiceActionResultCode.h"
#include "type/XmlAttribute.h"
#include "type/XmlAttributeCollection.h"
#include "type/XmlCData.h"
#include "type/XmlComment.h"
#include "type/XmlDeclaration.h"
#include "type/XmlDocument.h"
#include "type/XmlDocumentType.h"
#include "type/XmlElement.h"
#include "type/XmlNameTable.h"
#include "type/XmlNamespaceManager.h"
#include "type/XmlNode.h"
#include "type/XmlNodeList.h"
#include "type/XmlProcessingInstruction.h"
#include "type/XmlReadOptions.h"
#include "type/XmlText.h"
#include "type/XmlWriteOptions.h"

/// \file
/// \brief The door. One line, and an app has all of AL.
///
/// A GENERATED FILE INCLUDES THIS AND NOTHING ELSE FROM THE RUNTIME, because an AL file declares no
/// includes at all and the translation should not invent a list AL never wrote.
///
/// It costs nothing, and that was measured rather than assumed (2026-09-01, 200 generated table
/// headers, clang-19): with each file's own includes the pass takes 64 s and 62 s over two runs;
/// with the whole door forced into every file, 63 s and 63 s. The reason is in the preprocessor --
/// the whole door is 36 595 lines while a typical table header, which reaches for only 8 of the 17
/// type headers, already comes to 36 760. Some 36 500 of those are `<string_view>`, `<compare>`,
/// `<span>` and `<array>`, which every one of the door's headers pulls anyway. The door itself is
/// about 200 lines. There is nothing to save by including less of it.
///
/// The headers behind it stay one per AL type, and they stay reachable on their own: the runtime's
/// own sources include what they use, and a reader looking for `Date` finds `type/Date.h`.
///
/// \note The paths here are quoted rather than angled, which is what keeps the short names safe. A
///       quoted include is looked for beside the INCLUDING file first, so `type/Date.h` resolves
///       under `include/` before any `-I` on the command line is consulted -- and an app is
///       compiled with `-Iinclude -Iapps/<app>` at once.
