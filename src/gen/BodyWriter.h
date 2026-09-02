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

  /// \brief Whether a name is an object HANDLE rather than a value.
  ///
  /// \param name The AL name.
  /// \return True when reaching through it needs `->`.
  ///
  /// \note AN OBJECT MEMBER IS A HANDLE AND AL SAYS SO. Two codeunits may name each other and a
  ///       table may declare a variable of its own type -- `Currency Exchange Rate` does -- so an
  ///       eager member would be a class of infinite size, in C++ and in AL alike. The member is
  ///       created on first use, which C++ spells `->` (board:0037). A procedure's LOCAL stays a
  ///       value: it lives inside a body, where no cycle can form.
  [[nodiscard]] virtual bool IsHandle(std::string_view name) const {
    static_cast<void>(name);
    return false;
  }
};

std::string WriteStatements(const Names &scope, const std::vector<al::Stmt> &body, int indent);

std::string WriteSource(const al::TableObject &table,
                        const std::string &sourcePath,
                        const Objects &objects);

} // namespace agiru::gen
