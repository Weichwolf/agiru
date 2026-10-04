#pragma once

#include "Ast.h"
#include "EnumWriter.h"

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::gen {

struct TableRef {
  std::string identifier;
  std::string header;

  std::int32_t id = 0;

  std::map<std::string, std::string> fields;
  std::map<std::string, std::string> procedures;

  std::map<std::string, std::string> parts;

  std::string name;
  std::vector<std::pair<std::string, std::string>> dataItems;
  std::vector<std::string> requestFields;

  std::map<std::string, std::pair<std::string, std::string>> columnSources;

  std::set<std::string> interfaceReturns;

  std::set<std::string> tryFunctions;

  std::vector<al::ProcedureDecl> procedureDeclarations;
  std::vector<std::string> interfaceBases{};
  std::string declarationAssertions{};
  bool native = false;
};

const TableRef *ReachOf(const al::VarDecl &declared, const Objects &objects);

using TableIndex = std::map<std::string, TableRef>;

using DotNetUse = std::map<std::string, std::set<std::string>>;

struct CodeunitHeader {
  std::string text;
  std::vector<std::string> unresolvedTables;

  DotNetUse dotnet;

  DotNetUse absent;
};

using FieldEnums = std::map<std::string, std::map<std::string, std::string>>;

struct Objects {
  TableIndex tables;
  TableIndex reports;
  TableIndex xmlports;
  TableIndex queries;
  TableIndex codeunits;
  TableIndex interfaces;
  TableIndex pages;
  EnumIndex enums;
  FieldEnums fieldEnums;
  std::span<const al::TableObject> nativeTables;

  std::string module;
  std::string moduleHeader;
};

const TableIndex &PageIndexFor(const Objects &objects, std::string_view type);

[[nodiscard]] std::optional<std::int32_t> NativeTableNumberOf(const Objects &objects,
                                                              std::string_view name);

[[nodiscard]] bool
IsTryFunctionOf(const Objects &objects, const al::VarDecl *declared, std::string_view name);

std::vector<std::string> LentParametersOf(const std::vector<al::ProcedureDecl> &procedures,
                                          std::string_view name,
                                          const Objects &objects,
                                          const std::string &owner);

std::vector<std::string>
MemberLentParametersOf(const Objects &objects, const al::VarDecl *receiver, std::string_view name);

void NoteObjectNames(const Objects &objects);

struct QueryColumn {
  bool isColumn = false;
  std::string spelling;
};

[[nodiscard]] QueryColumn
QueryColumnOf(const Objects &objects, const al::VarDecl *declared, std::string_view member);

[[nodiscard]] std::string QueryColumnEnumeration(const Objects &objects,
                                                 const al::VarDecl *declared,
                                                 std::string_view member);

[[nodiscard]] std::string
FieldEnumerationOf(const Objects &objects, std::string_view table, std::string_view field);

std::string OptionTypeName(const std::string &owner,
                           const std::string &within,
                           const al::VarDecl &declared,
                           const std::vector<al::ProcedureDecl> &procedures);

[[nodiscard]] TableIndex PlatformTables();
[[nodiscard]] bool NeedsNativeDefinition(const TableRef &binding);
[[nodiscard]] TableIndex PlatformTables(std::span<const al::TableObject> declarations);
[[nodiscard]] FieldEnums PlatformFieldEnums(std::span<const al::TableObject> declarations,
                                            const TableIndex &tables);

[[nodiscard]] bool NamesAbsentType(const al::VarDecl &declared);

[[nodiscard]] std::string AbsentDotNetOf(const al::VarDecl &declared);

[[nodiscard]] FieldEnums PlatformFieldEnums();

struct InterfaceOutput {
  std::string text;
  std::string source;

  DotNetUse absent;
  DotNetUse dotnet;
};

InterfaceOutput WriteInterface(const al::InterfaceObject &object,
                               const std::string &sourcePath,
                               const Objects &objects);

CodeunitHeader WriteCodeunit(const al::CodeunitObject &unit,
                             const std::string &sourcePath,
                             const Objects &objects);

std::string WriteCodeunitSource(const al::CodeunitObject &unit,
                                const std::string &sourcePath,
                                const Objects &objects);

std::string CodeunitHeaderPath(const al::CodeunitObject &unit);

bool IsPublisher(const al::ProcedureDecl &procedure);

bool IsIsolatedPublisher(const al::ProcedureDecl &procedure);

std::string RaisingBody(const al::ProcedureDecl &procedure,
                        std::string_view kind,
                        const std::string &objectId,
                        const std::string &objectName);

std::string InlineOptionsOf(const std::string &owner,
                            const std::string &space,
                            const std::vector<al::VarDecl> &variables,
                            const std::vector<al::ProcedureDecl> &procedures,
                            const std::map<std::string, std::vector<std::string>> &already = {});

std::string ProcedureDeclaration(const al::ProcedureDecl &procedure,
                                 const Objects &objects,
                                 const std::string &owner,
                                 const std::set<std::string> &shadowed = {},
                                 const std::vector<al::ProcedureDecl> &all = {},
                                 const std::string &spelled = {});

bool IsTestCodeunit(const al::CodeunitObject &unit);

bool DeclaresAnOption(const std::vector<al::VarDecl> &variables,
                      const std::vector<al::ProcedureDecl> &procedures);

bool IsTryFunction(const al::ProcedureDecl &procedure);

bool IsTryFunction(std::span<const al::ProcedureDecl> procedures, std::string_view name);

bool DefaultsToTrue(const al::ProcedureDecl &procedure);

std::set<std::string> Shadowing(std::span<const al::VarDecl> variables,
                                std::span<const al::ProcedureDecl> procedures,
                                std::span<const al::LabelDecl> labels);

struct Spelling {
  std::string spelled;

  std::optional<std::string> body;
};

std::string ProcedureSignature(const al::ProcedureDecl &procedure,
                               const Objects &objects,
                               const std::string &owner,
                               const std::string &qualifier,
                               bool named,
                               const std::set<std::string> &shadowed = {},
                               const std::vector<al::ProcedureDecl> &all = {},
                               const Spelling &how = {});

[[nodiscard]] std::string MemberDeclarations(const std::string &owner,
                                             const std::vector<al::VarDecl> &variables,
                                             const std::vector<al::LabelDecl> &labels,
                                             const std::vector<al::ProcedureDecl> &procedures,
                                             const Objects &objects);

std::string ProcedureLocals(const al::ProcedureDecl &procedure,
                            const Objects &objects,
                            const std::string &owner,
                            const std::vector<al::ProcedureDecl> &all = {},
                            const std::set<std::string> &shadowed = {},
                            const std::string &body = {});

[[nodiscard]] std::string BodyIncludes(const std::string &text, const Objects &objects);

[[nodiscard]] std::string
DeclaredEnumMember(const Objects &objects, std::string_view enumeration, std::string_view member);

std::string SourceIncludesOf(const std::vector<al::VarDecl> &variables,
                             const std::vector<al::ProcedureDecl> &procedures,
                             const Objects &objects);

std::string QualifiedType(const std::string &type, const std::set<std::string> &names);

std::string DeclaredType(const al::VarDecl &declared, const Objects &objects);

void GatherAbsentIn(const std::vector<al::VarDecl> &variables,
                    const std::vector<al::ProcedureDecl> &procedures,
                    const Objects &objects,
                    DotNetUse &dotnet,
                    DotNetUse &absent);

bool NamesAbsentIn(const std::vector<al::VarDecl> &variables,
                   const std::vector<al::ProcedureDecl> &procedures,
                   const Objects &objects);

bool DeclaresAnObject(const al::VarDecl &declared);

bool ArityFits(const std::vector<al::ProcedureDecl> &procedures,
               std::string_view name,
               std::size_t count);

const TableRef *ReachObject(const al::VarDecl &declared, const Objects &objects);

}
