#pragma once

#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Text.h"

/// \file
/// \brief AL `TestFilter` -- the filters a test sets on a page.

namespace agiru {

/// \brief AL `TestFilter` -- what `TestPage.Filter` gives back.
///
/// \note IT IS REACHED WITHOUT PARENTHESES. AL writes `Page.FILTER.SETFILTER("No.", '1000')`, so
///       `Filter` is a member of the page and this is its type -- which is why the documentation
///       gives it a directory of its own (`methods-auto/testfilter/`) rather than listing these
///       under `TestPage`.
class TestFilter {
public:
  /// \brief AL `TestFilter.SetFilter(Field, Filter)`.
  /// \tparam Field The control the filter names.
  /// \param field  The control.
  /// \param filter The filter expression.
  /// \throws Error until a page runs (board:0030).
  template <typename Field> void SetFilter(const Field &field, std::string_view filter) {
    static_cast<void>(field);
    static_cast<void>(filter);
    Unfiltered();
  }

  /// \brief AL `TestFilter.GetFilter(Field)`.
  /// \tparam Field The control the filter names.
  /// \param field  The control.
  /// \return The filter standing on it.
  /// \throws Error until a page runs (board:0030).
  template <typename Field> [[nodiscard]] Text<0> GetFilter(const Field &field) const {
    static_cast<void>(field);
    Unfiltered();
  }

  /// \brief AL `TestFilter.SetCurrentKey(...)`.
  /// \tparam Fields The key's controls.
  /// \param fields  The controls, in key order.
  /// \return Whether the page carries that key.
  /// \throws Error until a page runs (board:0030).
  template <typename... Fields> Boolean SetCurrentKey(const Fields &...fields) {
    (static_cast<void>(fields), ...);
    Unfiltered();
  }

  /// \brief AL `TestFilter.CurrentKey()`.
  /// \return The key the page reads by.
  /// \throws Error until a page runs (board:0030).
  [[nodiscard]] Text<0> CurrentKey() const { Unfiltered(); }

  /// \brief AL `TestFilter.Ascending(Ascending)`.
  /// \param Ascending Which way to read.
  /// \return Which way it now reads.
  /// \throws Error until a page runs (board:0030).
  Boolean Ascending(Boolean Ascending) {
    static_cast<void>(Ascending);
    Unfiltered();
  }

  /// \brief AL `TestFilter.Ascending()`.
  /// \return Which way the page reads.
  /// \throws Error until a page runs (board:0030).
  [[nodiscard]] Boolean Ascending() const { Unfiltered(); }

private:
  /// \note NOT STATIC, because it will name the page. A filter belongs to one, and the message a
  ///       test sees is worth more than the byte the pointer costs.
  [[noreturn]] void Unfiltered() const {
    throw Error(page_ == nullptr ? "a TestPage's filters need a running page (board:0030)"
                                 : "this TestPage's filters are not implemented yet (board:0030)");
  }

  const void *page_ = nullptr;
};

}
