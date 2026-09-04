#pragma once

#include "Ast.h"
#include "EnumWriter.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace agiru::gen {

struct TableRef {
  std::string identifier;
  std::string header;

  std::map<std::string, std::string> fields;
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
  TableIndex codeunits;
  TableIndex interfaces;
  TableIndex pages;
  EnumIndex enums;
  FieldEnums fieldEnums;
};

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

std::string InlineOptionsOf(const std::string &owner,
                            const std::string &space,
                            const std::vector<al::VarDecl> &variables,
                            const std::vector<al::ProcedureDecl> &procedures);

std::string ProcedureDeclaration(const al::ProcedureDecl &procedure,
                                 const Objects &objects,
                                 const std::string &owner,
                                 const std::set<std::string> &shadowed = {},
                                 const std::vector<al::ProcedureDecl> &all = {},
                                 const std::string &spelled = {});

std::set<std::string> Shadowing(const std::vector<al::VarDecl> &variables,
                                const std::vector<al::ProcedureDecl> &procedures,
                                const std::vector<al::LabelDecl> &labels);

std::string ProcedureSignature(const al::ProcedureDecl &procedure,
                               const Objects &objects,
                               const std::string &owner,
                               const std::string &qualifier,
                               bool named,
                               const std::set<std::string> &shadowed = {},
                               const std::vector<al::ProcedureDecl> &all = {},
                               const std::string &spelled = {});

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

const TableRef *ReachObject(const al::VarDecl &declared, const Objects &objects);

}
