#pragma once

#include "Ast.h"
#include "EnumWriter.h"

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace agiru::gen {

struct TableRef {
  std::string identifier;
  std::string header;

  std::int32_t id = 0;

  std::map<std::string, std::string> fields;
  std::map<std::string, std::string> procedures;
};

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
};

void NoteObjectNames(const Objects &objects);

std::string OptionTypeName(const std::string &owner,
                           const std::string &within,
                           const al::VarDecl &declared,
                           const std::vector<al::ProcedureDecl> &procedures);

[[nodiscard]] TableIndex PlatformTables();

[[nodiscard]] FieldEnums PlatformFieldEnums();

struct InterfaceHeader {
  std::string text;

  DotNetUse absent;
  DotNetUse dotnet;
};

InterfaceHeader WriteInterface(const al::InterfaceObject &object,
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

std::set<std::string> Shadowing(const std::vector<al::VarDecl> &variables,
                                const std::vector<al::ProcedureDecl> &procedures,
                                const std::vector<al::LabelDecl> &labels);

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
