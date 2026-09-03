#pragma once

#include "Ast.h"
#include "CodeunitWriter.h"
#include "Expr.h"

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

  [[nodiscard]] virtual std::string ExitValue() const { return {}; }

  [[nodiscard]] virtual bool IsRecord(std::string_view variable) const {
    static_cast<void>(variable);
    return false;
  }

  [[nodiscard]] virtual bool MembersAreCalls(std::string_view variable) const {
    static_cast<void>(variable);
    return false;
  }

  [[nodiscard]] virtual bool IsHandle(std::string_view name) const {
    static_cast<void>(name);
    return false;
  }
};

std::string WriteStatements(const Names &scope, const std::vector<al::Stmt> &body, int indent);

std::string
WriteSource(const al::TableObject &table, const std::string &sourcePath, const Objects &objects);

} // namespace agiru::gen
