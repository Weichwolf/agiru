#pragma once

#include "Ast.h"
#include "CodeunitWriter.h"
#include "Expr.h"

#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

/// What a bare identifier means WHERE IT STANDS.
///
/// The same word is a different thing in a table trigger and in a codeunit procedure: in the first
/// it is usually a field of the record the trigger belongs to, in the second a local, a parameter,
/// a codeunit variable or another procedure of the same object. The statement translator does not
/// need to know which -- it needs to know the C++ spelling -- so the two callers answer that
/// question and the translator asks it.
class Names {
public:
  Names() = default;
  Names(const Names &) = delete;
  Names(Names &&) = delete;
  Names &operator=(const Names &) = delete;
  Names &operator=(Names &&) = delete;
  virtual ~Names() = default;

  /// The C++ spelling of a bare identifier, or empty when this scope does not know the name.
  [[nodiscard]] virtual std::string Resolve(std::string_view name) const = 0;

  /// The enumeration that `Name::Member` scopes through, or empty when `Name` is not one here.
  [[nodiscard]] virtual std::string Enumeration(std::string_view name) const = 0;
};

std::string WriteStatements(const Names &scope, const std::vector<al::Stmt> &body, int indent);

std::string WriteSource(const al::TableObject &table,
                        const std::string &sourcePath,
                        const Objects &objects);

} // namespace agiru::gen
