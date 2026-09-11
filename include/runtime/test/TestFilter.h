#pragma once

#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/test/PageCore.h"
#include "type/Boolean.h"
#include "type/Text.h"

#include <concepts>
#include <string_view>

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
    if (core_ == nullptr) { Unfiltered(); }
    if constexpr (requires { field.Name(); }) {
      core_->SetControlFilter(field.Name(), filter);
    } else if constexpr (std::convertible_to<const Field &, std::string_view>) {
      core_->SetControlFilter(std::string_view(field), filter);
    } else {
      static_cast<void>(filter);
      throw Error("TestFilter.SetFilter names a page control, not a value (board:0030)");
    }
  }

  /// \brief AL `TestFilter.SetFilter(Field, Filter)` where the filter is a VALUE and not text --
  ///        `Filter.SetFilter("User Security ID", User."User Security ID")` hands a Guid, and a
  ///        number, a date or an option arrive the same way.
  /// \tparam Field The control the filter names.
  /// \tparam Value Anything with a text form and no view of its own.
  /// \param field The control.
  /// \param value The value, rendered the way `Format` renders it.
  template <typename Field, typename Value>
    requires(!std::convertible_to<const Value &, std::string_view>) &&
            requires(const Value &v) { ::agiru::AsText(v); }
  void SetFilter(const Field &field, const Value &value) {
    SetFilter(field, std::string_view(::agiru::AsText(value)));
  }

  /// \brief Binds the filter pane to its page; `TestPage` does this when the page opens.
  /// \param core The page.
  void Bind(PageCore &core) { core_ = &core; }

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
  Boolean Ascending() const { Unfiltered(); }

private:
  PageCore *core_ = nullptr;

  /// \note NOT STATIC, because it will name the page. A filter belongs to one, and the message a
  ///       test sees is worth more than the byte the pointer costs.
  [[noreturn]] void Unfiltered() const {
    throw Error(page_ == nullptr ? "a TestPage's filters need a running page (board:0030)"
                                 : "this TestPage's filters are not implemented yet (board:0030)");
  }

  const void *page_ = nullptr;
};

}
