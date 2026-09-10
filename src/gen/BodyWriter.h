#pragma once

#include "Ast.h"
#include "CodeunitWriter.h"
#include "Expr.h"
#include "Names.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

struct OfVariable {
  std::string_view variable;
  std::string_view field;
};

class Names {
public:
  Names() = default;
  Names(const Names &) = delete;
  Names(Names &&) = delete;
  Names &operator=(const Names &) = delete;
  Names &operator=(Names &&) = delete;
  virtual ~Names() = default;

  [[nodiscard]] virtual std::string Resolve(std::string_view name) const = 0;

  [[nodiscard]] virtual std::string Enumeration(std::string_view name) const = 0;

  [[nodiscard]] virtual std::string FieldEnumeration(const OfVariable &field) const {
    static_cast<void>(field);
    return {};
  }

  [[nodiscard]] virtual std::string ObjectNamed(std::string_view kind,
                                                std::string_view name) const {
    return std::string(kind) + "::" + Identifier(name);
  }

  [[nodiscard]] virtual std::string EnumObject(std::string_view name) const {
    static_cast<void>(name);
    return {};
  }

  [[nodiscard]] virtual std::string ExitValue() const { return {}; }

  [[nodiscard]] virtual std::string ReturnedType() const { return {}; }

  [[nodiscard]] virtual bool IsRecord(std::string_view variable) const {
    static_cast<void>(variable);
    return false;
  }

  [[nodiscard]] virtual std::string MemberSpelling(const OfVariable &member) const {
    return Identifier(member.field);
  }

  [[nodiscard]] virtual bool HasField(const OfVariable &member) const {
    static_cast<void>(member);
    return false;
  }

  [[nodiscard]] virtual bool MemberIsCall(const OfVariable &member) const {
    return MembersAreCalls(member.variable);
  }

  [[nodiscard]] virtual bool MembersAreCalls(std::string_view variable) const {
    static_cast<void>(variable);
    return false;
  }

  [[nodiscard]] virtual std::string Module() const { return {}; }

  [[nodiscard]] virtual bool CallReturnsAHandle(std::string_view variable,
                                                std::string_view procedure) const {
    static_cast<void>(variable);
    static_cast<void>(procedure);
    return false;
  }

  [[nodiscard]] virtual bool ReturnsAHandle(std::string_view procedure) const {
    static_cast<void>(procedure);
    return false;
  }

  [[nodiscard]] virtual std::string ThisTable() const { return {}; }

  [[nodiscard]] virtual std::string BareRecordCall(std::string_view name) const {
    static_cast<void>(name);
    return {};
  }

  [[nodiscard]] virtual std::string ProcedureOf(const OfVariable &member) const {
    static_cast<void>(member);
    return {};
  }

  [[nodiscard]] virtual std::string AbsentDotNet(std::string_view name) const {
    static_cast<void>(name);
    return {};
  }

  [[nodiscard]] virtual bool AbsentControl(std::string_view name) const {
    static_cast<void>(name);
    return false;
  }

  [[nodiscard]] virtual std::string TableOf(std::string_view variable) const {
    static_cast<void>(variable);
    return {};
  }

  [[nodiscard]] virtual std::string EnumMember(std::string_view enumeration,
                                               std::string_view member) const {
    static_cast<void>(enumeration);
    return EnumeratorName(member);
  }

  [[nodiscard]] virtual std::vector<bool> VarParametersOfPublisher(std::string_view name) const {
    static_cast<void>(name);
    return {};
  }

  [[nodiscard]] virtual bool IsVariable(std::string_view name) const {
    static_cast<void>(name);
    return false;
  }

  [[nodiscard]] virtual bool IsHandle(std::string_view name) const {
    static_cast<void>(name);
    return false;
  }

  [[nodiscard]] virtual bool IsLabel(std::string_view name) const {
    static_cast<void>(name);
    return false;
  }

  [[nodiscard]] virtual bool IsTryFunction(std::string_view name) const {
    static_cast<void>(name);
    return false;
  }

  [[nodiscard]] virtual std::string DeclaredEnum(std::string_view variable) const {
    static_cast<void>(variable);
    return {};
  }

  [[nodiscard]] virtual std::string DeclaredType(std::string_view variable) const {
    static_cast<void>(variable);
    return {};
  }

  [[nodiscard]] virtual std::vector<std::string> LentParameters(std::string_view name) const {
    static_cast<void>(name);
    return {};
  }

  [[nodiscard]] virtual std::vector<std::string> ParameterTypes(std::string_view name) const {
    static_cast<void>(name);
    return {};
  }

  [[nodiscard]] virtual bool TakesArguments(std::string_view name, std::size_t count) const {
    static_cast<void>(name);
    static_cast<void>(count);
    return true;
  }
};

std::string WriteStatements(const Names &scope, const std::vector<al::Stmt> &body, int indent);

std::string
WriteSource(const al::TableObject &table, const std::string &sourcePath, const Objects &objects);

[[nodiscard]] std::string WriteDefinitions(const al::TableObject &table,
                                           const std::string &sourcePath,
                                           const Objects &objects);

[[nodiscard]] std::string FallsOffEnd(const al::ProcedureDecl &procedure, const Names &names);

[[nodiscard]] std::string QueryProcedureBodies(const al::TableObject &facade,
                                               const std::string &className,
                                               const Objects &objects);

[[nodiscard]] std::string WriteSource(const al::PageObject &page,
                                      const std::string &sourcePath,
                                      const Objects &objects,
                                      const al::TableObject *source);

[[nodiscard]] std::string WriteDefinitions(const al::PageObject &page,
                                           const std::string &sourcePath,
                                           const Objects &objects,
                                           const al::TableObject *source);

[[nodiscard]] std::string ControlTrigger(std::string_view trigger,
                                         std::string_view control,
                                         const std::vector<al::ProcedureDecl> &procedures = {});

}
