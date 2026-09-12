#pragma once

#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/test/Handlers.h"
#include "runtime/test/PageCore.h"
#include "runtime/test/TestAction.h"
#include "type/Action.h"
#include "type/Boolean.h"
#include "type/Dictionary.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/PageBackgroundTaskErrorLevel.h"
#include "type/PageStyle.h"
#include "type/PromptMode.h"
#include "type/Text.h"
#include "type/Variant.h"

#include <array>
#include <concepts>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

/// \file
/// \brief The base every generated AL page stands on, and the controls a page is made of.

namespace agiru {

/// \brief The declaration belonging to a generated page.
///
/// \tparam T The generated page class.
///
/// The generator specialises this beside the class, so the class itself carries nothing but what AL
/// wrote -- its controls, its actions, its variables and its procedures. The number, the name and
/// the page type live here, the way a table's field table does.
template <typename T> struct PageTraits;

/// \brief A call through a control whose object this build does not carry: a `usercontrol`'s
///        add-in, or a `part` whose page is outside the translated scope.
/// \param what The call as AL wrote it, `Part.Page().Method`.
/// \return Never; the type exists so the call can stand in an expression.
/// \throws Error always, naming the call -- a HOLE WITH A NAME rather than a compile error in a
///         page every test of that table needs (board:0034).
[[noreturn]] inline RefusedOptionValue RefusedControl(std::string_view what) {
  throw Error("the control " + std::string(what) + " has no object behind it in this build " +
              "(board:0034)");
}

/// \brief What a call through a control with nothing behind it answers when the call is NOT
///        refused: nothing, and the caller's default wherever a value is read.
///
/// \note A `usercontrol` RUNS IN THE CLIENT, and a headless session has none: BC's own test
///       framework runs an add-in method as a no-op (`BarcodeControl.RequestBarcodeAsync`,
///       `BusinessChart.SetValue`), so refusing it stopped a page every test of that table needs.
///       A PART WHOSE PAGE IS CARVED OUT OF THE SCOPE (`scope.json`, a decision: `Power BI
///       Embedded Report Part` sits in the excluded `System.Integration.PowerBI`) is treated the
///       same -- a part with nothing behind it shows nothing, and `Job List.OnOpenPage`'s
///       `SetPageContext` on it did nothing worth stopping 9 UT cases for (2026-09-12). The
///       predecessor answered the same with its `_NilValue` sentinel.
class AbsentControlValue {
public:
  /// \brief The default of whatever type reads it. \tparam T The type. \return `T{}`.
  template <typename T>
    requires(!std::is_same_v<T, std::string> && !std::is_same_v<T, std::string_view>)
  operator T() const {
    return T{};
  }

  /// \brief A call chained on the answer answers the same. \tparam A The arguments.
  /// \return Another absent answer.
  template <typename... A> AbsentControlValue operator()(const A &...) const { return {}; }

  /// \brief AL `if Control.X() then`: false. \return False.
  explicit operator bool() const { return false; }
};

/// \brief A call through a control with nothing behind it: a no-op, see `AbsentControlValue`.
/// \param what The call as AL wrote it, `Part.Page().Method`, for a trace.
/// \return An absent answer.
inline AbsentControlValue AbsentControl(std::string_view what) {
  static_cast<void>(what);
  return {};
}

/// \brief One control's triggers, as a page's `PageTraits` tabulates them.
///
/// \tparam P The generated page class.
///
/// \note IT IS STATIC DATA BESIDE THE PAGE, the way a table's `kOnValidate` is: the generator
///       emits one row per control that declares a trigger, with a member pointer per trigger
///       it declares and `nullptr` for the rest. A headless page (`TestPage`) finds the control
///       by its AL name and calls through the pointer; nothing is looked up by string at run
///       time beyond the one name the test wrote.
template <typename P> struct ControlTrigger {
  std::string_view control;          ///< The control's AL name.
  void (P::*validate)() = nullptr;   ///< `OnValidate`.
  void (P::*action)() = nullptr;     ///< `OnAction`.
  void (P::*drillDown)() = nullptr;  ///< `OnDrillDown`.
  void (P::*assistEdit)() = nullptr; ///< `OnAssistEdit`.
  /// \brief `OnLookup(var Text: Text): Boolean` -- the trigger takes the control's text and
  ///        answers whether it set a new one, which the harness then validates as a `SetValue`
  ///        (`testfield-lookup--method.md`). 552 BaseApp controls declare it (2026-09-09).
  ::agiru::Boolean (P::*lookup)(::agiru::Text<0> &) = nullptr;
  /// \brief The `Visible` property when it is an EXPRESSION rather than a literal -- the parser
  ///        turns `Visible = AmountVisible` into a trigger that computes it, so the harness reads
  ///        what the page's variables say now and not what the property said at translation.
  ::agiru::Boolean (P::*visible)() = nullptr;
  ::agiru::Boolean (P::*enabled)() = nullptr;  ///< \see visible
  ::agiru::Boolean (P::*editable)() = nullptr; ///< \see visible
  /// \brief Sets the control's page VARIABLE from text, for a control whose source is one
  ///        (`SourceExpr = JobSourceType`): what `TestField.SetValue` does when there is no field.
  void (*set)(P &page, std::string_view text) = nullptr;
  /// \brief Reads the control's page variable as text, the same way.
  std::string (*text)(const P &page) = nullptr;
};

/// \brief What every AL page can do, without the generated class saying any of it.
///
/// \tparam Derived The generated page class.
///
/// A page is the object BC's user actually works in, and AL code drives it from two sides: the
/// application calls `Page.Run` and `Page.RunModal`, and a test drives every control through
/// `TestPage`. Both halves are the platform's, so both live here.
/// \brief What every AL page can do, without the generated class saying any of it.
/// \tparam Derived The generated page class.
/// \note THE PARAMETER DEFAULTS TO `void`, WHICH IS WHAT LETS AL'S OWN SPELLING THROUGH. AL writes
///       `Page.RunModal(Id, Rec)` -- a static on the TYPE -- and it writes `page 21 "Customer
///       Card"` whose generated class derives from `Page<Customer_Card>`. A class template and a
///       class of the same name cannot both exist in C++, so the statics live on the template and
///       the generator spells the static form `Page<>::RunModal`. `Id()` and `Name()` are the two
///       that need a Derived, and asking for them on `Page<>` is a compile error rather than a
///       wrong number.
/// \brief A PART control on a page: AL reaches the sub-page through it -- `CurrPage.Matrix.PAGE`.
///
/// \tparam P The sub-page's generated class.
///
/// \note THE PART IS A CONTROL AND THE SUB-PAGE IS AN OBJECT, and AL keeps them apart with
///       `.PAGE`. The part itself carries what the platform offers on a control (`Visible`,
///       `Editable`); everything on the other side of `.PAGE` is the sub-page's own surface, and
///       reaching it needs a running UI (board:0030).
namespace detail {
template <typename P> void OpenPage(P &page, bool editable, bool isNew);

/// \brief Calculates the FlowFields a page's controls show, the way the platform does BEFORE the
///        record's `OnAfterGetRecord` runs: a page trigger reads `Rec.Balance` calculated, and a
///        test's `AssertEquals` on that control reads the value and not zero.
/// \param record   The page's record, positioned.
/// \param table    Its declaration.
/// \param controls The page's layout, walked through every container.
/// \note A FORMULA OVER A TABLE OUTSIDE THE BUILD IS LEFT ALONE (`CalcFieldIfCarried`): the page
///       shows the field whether or not a test reads it, and refusing there would stop every
///       landing on the page.
void CalcShownFlowFields(void *record, const TableDef &table, std::span<const ControlDef> controls);

/// \brief The data caption a page composes from its record
/// (`devenv-datacaptionfields-property.md`):
///        a CARD takes the table's `DataCaptionFields` (its primary key when it declares none) from
///        the current record; a TABULAR page shows one only where a filter on a field the page
///        lists fixes a single value -- through the field's `TableRelation` to the related table's
///        own `DataCaptionFields`, or the value itself where there is no relation.
/// \param record The page's record.
/// \param table  Its declaration.
/// \param type   The page's `PageType`.
/// \param fields The page's `DataCaptionFields` property as AL wrote it, empty for none.
/// \return The data caption, empty when nothing composes one.
std::string
PageDataCaption(const void *record, const TableDef &table, PageType type, std::string_view fields);

/// \brief Raises one of the PAGE'S PLATFORM EVENTS (`devenv-event-types.md`, the page table):
///        `OnOpenPageEvent`, `OnAfterGetRecordEvent`, `OnAfterGetCurrRecordEvent`,
///        `OnNewRecordEvent`, `OnInsertRecordEvent`, `OnModifyRecordEvent`, `OnDeleteRecordEvent`,
///        `OnQueryClosePageEvent`, `OnClosePageEvent`, `OnBeforeActionEvent`,
///        `OnAfterActionEvent`. The runtime raises them, never the generated page, because they
///        fire whether or not the page declares the trigger -- `VAT Report Mgt.` sends its
///        notification from `VAT Return Period List`'s `OnOpenPageEvent`, `OAuth 2.0 Mgt.` from
///        `OAuth 2.0 Setup`'s `OnAfterGetCurrRecordEvent`, and neither page has the trigger
///        (4 UT cases, "Queue underflow", 2026-09-12). The BaseApp subscribes 38 times to
///        `OnModifyRecordEvent`, 20 to `OnOpenPageEvent`, 17 each to insert and delete.
/// \tparam P      The generated page class.
/// \tparam Values The event's argument types, `Rec` first.
/// \param page    The page, for its number and name.
/// \param event   The event's name.
/// \param element The action's name for the action events, empty otherwise.
/// \param names   The event's parameter names, the way the table documents them.
/// \param values  The arguments.
template <typename P, typename... Values>
void RaisePageEvent(P &page,
                    std::string_view event,
                    std::string_view element,
                    std::span<const std::string_view> names,
                    Values &...values) {
  static_cast<void>(page);
  if constexpr (requires {
                  PageTraits<P>::kId.Value();
                  PageTraits<P>::kName;
                }) {
    ::agiru::detail::RaiseEventOn(EventObject::Page,
                                  PageTraits<P>::kId.Value(),
                                  PageTraits<P>::kName,
                                  event,
                                  element,
                                  names,
                                  values...);
  }
}

/// \brief `RaisePageEvent` for the events that carry only `var Rec`.
/// \tparam P The generated page class.
/// \param page    The page.
/// \param event   The event's name.
/// \param element The action's name for the action events, empty otherwise.
template <typename P>
void RaisePageRecordEvent(P &page, std::string_view event, std::string_view element = {}) {
  if constexpr (requires { page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{}); }) {
    static constexpr std::array<std::string_view, 1> kNames{"Rec"};
    RaisePageEvent(page, event, element, kNames, page.Rec);
  }
}

/// \brief Whether a property's text says `false`, case folded the way AL reads it.
/// \param property The property text, empty where AL declares none.
/// \return True only for `false`.
[[nodiscard]] constexpr bool SaysFalse(std::string_view property) {
  if (property.size() != 5) { return false; }
  constexpr std::string_view kFalse = "false";
  for (std::size_t i = 0; i < kFalse.size(); ++i) {
    const char c = property[i];
    const char lower = c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c;
    if (lower != kFalse[i]) { return false; }
  }
  return true;
}

/// \brief Puts a page's `SourceTableView` on its record, as FILTER GROUP 2.
///
/// \param record The page's record.
/// \param table  Its declaration.
/// \param view   The `SourceTableView` property, as AL wrote it; nothing happens when empty.
///
/// \note GROUP 2 IS WHERE THE PLATFORM PUTS IT (`record-filtergroup-method.md`, `Form`), so what a
///       caller set in group 0 survives and `GetFilters` still answers what the caller asked. A
///       SUBFORM carries its own view beside the `SubPageLink`, and that is where a sales line's
///       `Document Type` comes from: the link names only `Document No.`.
void ApplyPageView(void *record, const TableDef &table, std::string_view view);

/// \brief Gives a record a page opens NEW what the page's `SourceTableView` fixes.
///
/// \param record   The page's record, freshly `Init`ed.
/// \param table    Its declaration.
/// \param view     The `SourceTableView` property, as AL wrote it; nothing happens when empty.
/// \param allFields Whether fields outside the primary key are seeded too.
///
/// \note THE VIEW IS FILTER GROUP 2 AND NOT THE RECORD'S OWN FILTERS
///       (`record-filtergroup-method.md` tabulates `SourceTableView` and `DataItemTableView` under
///       group 2, `Form`). So `GetFilters` in group 0 is unchanged and a test that reads the
///       record's filters sees what it set itself.
///
/// \note IT SEEDS ONLY A PAGE THAT OPENS NEW. A page handed a record by its caller is positioned
///       on that row and takes nothing from the view -- applying the view on every open cost 12 UT
///       cases and flipped two number series (board:0696, chain 123).
void SeedNewPageRecord(void *record, const TableDef &table, std::string_view view, bool allFields);

/// \brief Applies a `SubPageLink` -- `Field = field(Other)`, `= const(Value)`, `= filter(...)` --
///        as filters on the subpage's record, read from the parent's current record.
/// \param sub The subpage's record.
/// \param subTable Its declaration.
/// \param parent The parent page's record.
/// \param parentTable Its declaration.
/// \param link The property text as the page declares it.
void ApplySubPageLink(void *sub,
                      const TableDef &subTable,
                      const void *parent,
                      const TableDef &parentTable,
                      std::string_view link);
}

template <typename P> class PartRef {
public:
  /// \brief AL `CurrPage.<Part>.PAGE` -- the sub-page behind the part, made and opened headless the
  ///        first time it is asked for.
  /// \return The sub-page.
  /// \note THE LINK IS NOT WIRED YET: `SubPageLink` narrows the sub-page's `Rec` to the parent's
  ///       row in BC, and here the sub-page opens over its whole table (board:0430). A procedure
  ///       called through the part runs; a value read through it is the first row's until then.
  [[nodiscard]] P &Page() const {
    P &page = *page_.operator->();
    if (!opened_) {
      opened_ = true;
      detail::OpenPage(page, true, false);
    }
    return page;
  }

  /// \brief The instance without opening it, for a test page that opens and links it itself and
  ///        then counts as the one that opened it.
  /// \return The subpage instance.
  [[nodiscard]] P &Held() const {
    opened_ = true;
    return *page_.operator->();
  }

  /// \brief AL `CurrPage.<Part>.Visible(Boolean)` -- sets whether the part shows.
  /// \param NewVisible Whether it shows.
  /// \return The value that stood before.
  ::agiru::Boolean Visible(::agiru::Boolean NewVisible) const {
    const ::agiru::Boolean was = visible_;
    visible_ = NewVisible;
    return was;
  }

  /// \brief AL `CurrPage.<Part>.Editable(Boolean)` -- sets whether the part takes input.
  /// \param NewEditable Whether it takes input.
  /// \return The value that stood before.
  ::agiru::Boolean Editable(::agiru::Boolean NewEditable) const {
    const ::agiru::Boolean was = editable_;
    editable_ = NewEditable;
    return was;
  }

private:
  mutable Instance<P> page_;
  mutable bool opened_ = false;
  mutable ::agiru::Boolean visible_ = true;
  mutable ::agiru::Boolean editable_ = true;
};

namespace detail {

/// \brief Runs the triggers a page owes after landing on a record: `OnAfterGetRecord`, then
///        `OnAfterGetCurrRecord`.
/// \tparam P The generated page class.
/// \param page The page.
template <typename P> void AfterGetRecord(P &page) {
  if constexpr (requires {
                  PageTraits<P>::kPage.layout;
                  page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{});
                }) {
    using Source = std::remove_cvref_t<decltype(page.Rec)>;
    detail::CalcShownFlowFields(
        static_cast<void *>(&page.Rec), TableTraits<Source>::kTable, PageTraits<P>::kPage.layout);
  }
  if constexpr (requires { page.OnAfterGetRecord(); }) { page.OnAfterGetRecord(); }
  RaisePageRecordEvent(page, "OnAfterGetRecordEvent");
  if constexpr (requires { page.OnAfterGetCurrRecord(); }) { page.OnAfterGetCurrRecord(); }
  RaisePageRecordEvent(page, "OnAfterGetCurrRecordEvent");
}

/// \brief Starts a new record on a page the way the platform does: the page notes it, then
///        `OnNewRecord` runs.
/// \tparam P The generated page class.
/// \param page The page, whose `Rec` was just initialised.
/// \param belowXRec What `OnNewRecord` is told.
template <typename P> void StartNewRecord(P &page, bool belowXRec) {
  page.StartedNewRecord();
  if constexpr (requires { page.OnNewRecord(::agiru::Boolean{}); }) { page.OnNewRecord(belowXRec); }
  if constexpr (requires { page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{}); }) {
    static constexpr std::array<std::string_view, 3> kNames{"Rec", "BelowxRec", "xRec"};
    ::agiru::Boolean below = belowXRec;
    auto &before = page.Rec.StoredImage();
    RaisePageEvent(page, "OnNewRecordEvent", {}, kNames, page.Rec, below, before);
  }
}

/// \brief Opens a page the way the platform does: `OnInit`, the record positioned (or a new one
///        with `OnNewRecord`), `OnOpenPage`, then the after-get triggers.
/// \note THE RECORD `Page.Run(Rec)` PASSED IS THE ONE SHOWN, when it exists in the set:
///       `page-run-integer-table-joker-method.md` -- "Use this optional parameter to select a
///       specific record to display on the page. When the record is displayed, the key and
///       filters attached to the record are used." So the page finds THAT row first and falls
///       back to the first of the filtered set only when it is not there -- a list opened on
///       the third period drilled down into the first one before (24 UT cases, 2026-09-09).
/// \tparam P The generated page class.
/// \param page     The page.
/// \param editable Whether it opens for editing; a page whose `Editable` property is `false`
///                 opens for viewing whatever the caller asked (`devenv-editable-property.md`), so
///                 `TestPage.Editable()`, every control's `Editable()` and `CurrPage.Editable`
///                 answer false on `VAT Return Period Card` (5 UT cases, 2026-09-12).
/// \param isNew    Whether it opens on a new record (`OpenNew`).
template <typename P> void OpenPage(P &page, bool editable, bool isNew) {
  bool opensEditable = editable;
  if constexpr (requires { PageTraits<P>::kPage.editable; }) {
    opensEditable = editable && !detail::SaysFalse(PageTraits<P>::kPage.editable);
  }
  page.OpenedAs(opensEditable);
  if constexpr (requires { page.OnInit(); }) { page.OnInit(); }
  bool found = false;
  if constexpr (requires { page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{}); }) {
    using Source = std::remove_cvref_t<decltype(page.Rec)>;
    auto &platform = static_cast<typename Source::Platform_Half &>(page.Rec);
    if (isNew) {
      platform.Init();
      if constexpr (requires { PageTraits<P>::kPage; }) {
        detail::SeedNewPageRecord(static_cast<void *>(&page.Rec),
                                  TableTraits<Source>::kTable,
                                  PageTraits<P>::kPage.sourceTableView,
                                  true);
      }
      StartNewRecord(page, false);
    } else {
      if constexpr (requires { PageTraits<P>::kPage; }) {
        detail::ApplyPageView(static_cast<void *>(&page.Rec),
                              TableTraits<Source>::kTable,
                              PageTraits<P>::kPage.sourceTableView);
      }
    }
  }
  if constexpr (requires { page.OnOpenPage(); }) { page.OnOpenPage(); }
  RaisePageRecordEvent(page, "OnOpenPageEvent");
  if constexpr (requires { page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{}); }) {
    if (!isNew) {
      using Source = std::remove_cvref_t<decltype(page.Rec)>;
      auto &platform = static_cast<typename Source::Platform_Half &>(page.Rec);
      found = static_cast<bool>(platform.Find("=")) || static_cast<bool>(platform.FindFirst());
    }
  }
  if (found) { page.LandedOnRecord(); }
  if (found || isNew) { AfterGetRecord(page); }
}

/// \brief Closes a page the way the platform does: `OnQueryClosePage`, then `OnClosePage`.
/// \tparam P The generated page class.
/// \param page The page.
/// \throws Error when `OnQueryClosePage` refuses.
template <typename P> void ClosePage(P &page) {
  if constexpr (requires(::agiru::Action action) {
                  { page.OnQueryClosePage(action) } -> std::convertible_to<bool>;
                }) {
    if (!page.OnQueryClosePage(page.ClosedWith())) {
      throw Error("the page refused to close (OnQueryClosePage)");
    }
  }
  if constexpr (requires { page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{}); }) {
    static constexpr std::array<std::string_view, 2> kNames{"Rec", "AllowClose"};
    ::agiru::Boolean allowClose = true;
    RaisePageEvent(page, "OnQueryClosePageEvent", {}, kNames, page.Rec, allowClose);
    if (!allowClose) { throw Error("the page refused to close (OnQueryClosePageEvent)"); }
  }
  if constexpr (requires { page.OnClosePage(); }) { page.OnClosePage(); }
  RaisePageRecordEvent(page, "OnClosePageEvent");
}

/// \brief Hands the record a `Page.Run(Rec)` names to the page: its filters, and its position.
/// \tparam P      The generated page class.
/// \tparam Record What was passed; only the page's own source table is taken.
/// \param page   The page.
/// \param record The argument.
/// \warning AN `Instance<T>` IS A HANDLE AND NOT THE RECORD: a codeunit's global record reaches
///          `Page.Run` as one, and taking the handle's own address as the record put a page's
///          `Rec.Copy` onto garbage (SIGSEGV in three UT codeunits, found under the address
///          sanitizer 2026-09-09). Every taker here dereferences a handle first.
template <typename P, typename Record> void AdoptRecord(P &page, const Record &record) {
  if constexpr (requires { record.operator->(); }) {
    AdoptRecord(page, *record.operator->());
  } else if constexpr (requires {
                         page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{});
                         page.Rec.Copy(record);
                       }) {
    using Source = std::remove_cvref_t<decltype(page.Rec)>;
    static_cast<typename Source::Platform_Half &>(page.Rec).Copy(record);
  } else {
    static_cast<void>(record);
  }
}

/// \brief AL `Page.Run(Rec)` / `Page.RunModal(Rec)` on a generated page, headless.
///
/// \tparam P         The generated page class.
/// \tparam Arguments The record, when one was passed.
/// \param modal     Whether it is `RunModal`.
/// \param arguments The record, when one was passed.
/// \return The action the page closed with.
/// \throws Error when no test harness answers: AL's `Unhandled UI` for a page a test did not
///         trap or declare a handler for.
///
/// \note THE PAGE RUNS FOR WHOEVER CATCHES IT. A non-modal run goes to a `TestPage.Trap()` first,
///       and the harness then owns the page and drives it; otherwise a `[PageHandler]` or
///       `[ModalPageHandler]` for this page number is invoked with the page, already opened, and
///       the page closes when the handler returns. There is no third case yet: a page nobody
///       waits for is an unhandled UI, which is what AL says too (board:0030).
/// \brief AL's caller sees the page's record when the page closes: `if Page.RunModal(0, Item) =
///        Action::LookupOK then ... Item."No."` reads what the user picked, because the record
///        went in by `var`. A handle (`Instance<T>`) reaches through; a const argument stays as it
///        was; a record of another table is left alone.
/// \tparam P      The generated page class.
/// \tparam Record What was handed to `Run`.
/// \param page   The page that ran.
/// \param record The caller's record.
template <typename P, typename Record> void GiveBackRecord(P &page, Record &record) {
  if constexpr (std::is_const_v<Record>) {
    static_cast<void>(page);
  } else if constexpr (requires { record.operator->(); }) {
    GiveBackRecord(page, *record.operator->());
  } else if constexpr (requires {
                         page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{});
                         record = page.Rec;
                       }) {
    record = page.Rec;
  } else {
    static_cast<void>(page);
  }
}

/// \brief Runs an OPENED page through the test handler for it, and closes it.
/// \tparam P The generated page class.
/// \param page  The page, already opened.
/// \param modal Whether it is `RunModal`.
/// \return The action the page closed with.
/// \throws Error "Unhandled UI: ModalPage X" when no handler is installed, which is BC's wording.
template <typename P>::agiru::Action RunHandled(P &page, bool modal) {
  const std::int32_t id = PageTraits<P>::kId.Value();
  const TestHandler *handler =
      HandlerTable::For(modal ? HandlerKind::ModalPage : HandlerKind::Page, id);
  if (handler == nullptr) {
    throw Error(std::string("Unhandled UI: ") + (modal ? "ModalPage " : "Page ") +
                std::string(PageTraits<P>::kName));
  }
  if (modal) { page.RunsModally(); }
  handler->invoke(PageTraits<P>::kName, &page);
  HandlerTable::Ran(*handler);
  ClosePage(page);
  return page.ClosedWith();
}

template <typename P, typename... Arguments>
::agiru::Action RunPage(bool modal, Arguments &&...arguments) {
  auto page = std::make_unique<P>();
  (AdoptRecord(*page, arguments), ...);
  OpenPage(*page, true, false);
  const std::int32_t id = PageTraits<P>::kId.Value();
  if (!modal && ReleaseTrap(id, page.get(), true)) {
    static_cast<void>(page.release());
    return ::agiru::Action::OK;
  }
  const ::agiru::Action action = RunHandled(*page, modal);
  (GiveBackRecord(*page, arguments), ...);
  return action;
}

/// \brief AL `PageVariable.Run()` / `PageVariable.RunModal()`: the variable's OWN page object runs,
///        with the record, filters and lookup mode the caller put on it first
///        (`ItemList.SetTableView(Item); ItemList.LookupMode(true); if ItemList.RunModal() =
///        Action::LookupOK then ItemList.GetRecord(Item)` -- 1 216 such calls in the BaseApp).
/// \tparam P The generated page class.
/// \param page  The variable's page object.
/// \param modal Whether it is `RunModal`.
/// \return The action the page closed with; `OK` when a `Trap` took a non-modal run.
/// \warning A TRAPPED PAGE OUTLIVES THE VARIABLE THAT RAN IT. `WorkflowPage.Trap();
///          WorkflowsPage.NewAction.Invoke()` runs a page variable LOCAL to the action, and the
///          harness drove a freed object once the action returned (`WF Buffer Table/Page UT`
///          crashed with SIGSEGV in chain 86, 2026-09-10). So a pending trap takes a COPY the
///          harness owns, and only a page that cannot be copied is handed over unowned.
template <typename P>::agiru::Action RunInstance(P &page, bool modal) {
  OpenPage(page, true, false);
  if (!modal && TrapPending(PageTraits<P>::kId.Value())) {
    if constexpr (std::is_copy_constructible_v<P>) {
      auto *held = new P(page);
      if (ReleaseTrap(PageTraits<P>::kId.Value(), held, true)) { return ::agiru::Action::OK; }
      delete held;
    } else {
      if (ReleaseTrap(PageTraits<P>::kId.Value(), &page, false)) { return ::agiru::Action::OK; }
    }
  }
  return RunHandled(page, modal);
}

}

/// \brief Runs a generated page for the catalogue: `Page.Run(Number, Rec)` lands here.
/// \tparam P The generated page class.
/// \param modal  Whether it is `RunModal`.
/// \param record The record passed, or `nullptr`.
/// \param table  Its declaration, or `nullptr`.
/// \return The action the page closed with.
template <typename P>
::agiru::Action RunPageEntry(bool modal, void *record, const TableDef *table, bool writable) {
  if constexpr (requires(P &page) {
                  page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{});
                }) {
    using Source = std::remove_cvref_t<decltype(std::declval<P &>().Rec)>;
    if (record != nullptr && table != nullptr && table->id == TableTraits<Source>::kTable.id) {
      Source &source = *static_cast<Source *>(record);
      if (writable) { return detail::RunPage<P>(modal, source); }
      return detail::RunPage<P>(modal, std::as_const(source));
    }
  }
  static_cast<void>(record);
  static_cast<void>(table);
  static_cast<void>(writable);
  return detail::RunPage<P>(modal);
}

/// \brief The catalogue entry of a generated page.
/// \tparam P The generated page class.
template <typename P>
inline const PageEntry kPageEntry{.page = &PageTraits<P>::kPage, .run = &RunPageEntry<P>};

/// \brief Puts a generated page in the catalogue by existing, the way `RegisterTable` does.
/// \tparam P The generated page class.
template <typename P> struct RegisterPage {
  RegisterPage() { RegisterPageEntry(&kPageEntry<P>); }

  RegisterPage(const RegisterPage &) = delete;
  RegisterPage(RegisterPage &&) = delete;
  RegisterPage &operator=(const RegisterPage &) = delete;
  RegisterPage &operator=(RegisterPage &&) = delete;
  ~RegisterPage() = default;
};

namespace detail {

/// \brief `Page.Run(Number)` / `Page.RunModal(Number)`, with or without a record.
/// \tparam Arguments The record, when one was passed.
/// \param modal     Whether it is `RunModal`.
/// \param id        The page number.
/// \param arguments The record, when one was passed.
/// \return The action the page closed with.
/// \throws Error when this build carries no page of that number.
/// \note NUMBER 0 IS THE TABLE'S OWN LOOKUP PAGE: `page-run-integer-table-joker-method.md` --
///       "If you enter zero (0), the system displays the default lookup window for the current
///       page" -- so `PAGE.RunModal(0, Rec)` opens the record's `LookupPageId`, and the
///       `DrillDownPageId` where a table declares only that (16 UT cases, 2026-09-09).
template <typename... Arguments>
::agiru::Action RunPageByNumber(bool modal, ::agiru::Integer id, Arguments &&...arguments) {
  void *record = nullptr;
  const TableDef *table = nullptr;
  bool writable = false;
  const auto take = [&](auto &argument) {
    using A = std::remove_cvref_t<decltype(argument)>;
    constexpr bool kConst = std::is_const_v<std::remove_reference_t<decltype(argument)>>;
    if constexpr (requires {
                    argument.operator->();
                    TableTraits<A>::kTable;
                  }) {
      record = const_cast<void *>(static_cast<const void *>(argument.operator->()));
      table = &TableTraits<A>::kTable;
      writable = !kConst;
    } else if constexpr (requires { TableTraits<A>::kTable; }) {
      record = const_cast<void *>(static_cast<const void *>(&argument));
      table = &TableTraits<A>::kTable;
      writable = !kConst;
    }
  };
  (take(arguments), ...);
  if (id == 0) {
    if (table == nullptr) {
      throw Error("Page.Run(0) needs a record, whose table names the default lookup page");
    }
    id = table->lookupPageId.Value() != 0 ? table->lookupPageId.Value()
                                          : table->drillDownPageId.Value();
    if (id == 0) {
      throw Error("Page.Run(0): " + std::string(table->name) +
                  " declares no LookupPageId and no DrillDownPageId");
    }
  }
  const PageEntry *entry = FindPage(PageId{id});
  if (entry == nullptr) {
    throw Error("Page.Run(" + std::to_string(id) + "): this build carries no page of that number");
  }
  return entry->run(modal, record, table, writable);
}

}

template <typename Derived = void> class Page {
public:
  /// \brief Marks the page opened, in the mode a runner chose.
  /// \param editable Whether `OpenEdit`/`OpenNew` (true) or `OpenView` (false).
  void OpenedAs(bool editable) { editable_ = editable; }

  /// \brief Whether the page was opened for editing.
  /// \return True after `OpenEdit` or `OpenNew`.
  [[nodiscard]] bool OpenedEditable() const { return editable_; }

  /// \brief Records the action the page closed with (`OK`, `Cancel`, `Yes`, `No`, `LookupOK`).
  /// \param action The action.
  ///
  /// \note `OK` ON A LOOKUP WINDOW IS `LookupOK`, which `page-runmodal--method.md` states: "To
  ///       close a lookup window, the user chooses the OK button" returns `LookupOK`, and
  ///       `Cancel` there is `LookupCancel`. A page is a lookup window when `LookupMode` is on, and
  ///       when a collection-oriented page (`List`, `ListPart`, `Worksheet`) runs MODALLY -- the
  ///       documentation's own example is `SetRecord; if RunModal = Action::LookupOK then
  ///       GetRecord` with no `LookupMode` in sight, and `Price Source - Customer.IsLookupOK`
  ///       reads `Page.RunModal(Page::"Customer Lookup", Customer) = Action::LookupOK` the same
  ///       way (6 cases of `Price Source UT`, 2026-09-11). A Card or a dialog closes with `OK`.
  void CloseWith(::agiru::Action action) {
    bool lookup = static_cast<bool>(lookupMode_);
    if constexpr (requires { PageTraits<Derived>::kPage.type; }) {
      lookup = lookup || (modal_ && !EntityOriented(PageTraits<Derived>::kPage.type));
    }
    if (lookup && action == ::agiru::Action::OK) {
      action = ::agiru::Action::LookupOK;
    } else if (lookup && action == ::agiru::Action::Cancel) {
      action = ::agiru::Action::LookupCancel;
    }
    closeAction_ = action;
  }

  /// \brief Notes that the page runs modally, which is what makes a list a lookup window.
  void RunsModally() { modal_ = true; }

  /// \brief What `Page.RunModal` answers: the action the page closed with.
  /// \return The action; `OK` when nothing said otherwise.
  [[nodiscard]] ::agiru::Action ClosedWith() const { return closeAction_; }

  /// \brief The page's AL number.
  /// \return The number AL declared.
  [[nodiscard]] static constexpr PageId Id() { return PageTraits<Derived>::kId; }

  /// \brief The page's AL name.
  /// \return The name AL declared.
  [[nodiscard]] static constexpr std::string_view Name() { return PageTraits<Derived>::kName; }

  /// \brief AL `Page.Run(PageId [, Record])` -- shows the page.
  ///
  /// \tparam Arguments What AL handed it: nothing, or the record to open on.
  /// \param arguments The record, when there is one.
  /// \throws Error until the UI runs (board:0030).
  ///
  /// \note AL NAMES THE KIND TWICE AND THE GENERATED FORM ONCE. `Page.Run(Page::"X", Rec)` becomes
  ///       `pages::X::Run(Rec)`: the object is the receiver, which is what the call means.
  template <typename... Arguments>
    requires(sizeof...(Arguments) > 0)
  static void Run(Arguments &&...arguments) {
    if constexpr (std::is_void_v<Derived>) {
      static_cast<void>(detail::RunPageByNumber(false, arguments...));
    } else {
      static_cast<void>(detail::RunPage<Derived>(false, arguments...));
    }
  }

  /// \brief AL `PageVariable.Run()`: this variable's own page object runs, non-modally, with what
  ///        `SetRecord`, `SetTableView` and `LookupMode` put on it. A `TestPage.Trap()` takes it
  ///        without owning it -- the variable still does.
  void Run()
    requires(!std::is_void_v<Derived>)
  {
    static_cast<void>(detail::RunInstance(static_cast<Derived &>(*this), false));
  }

  /// \brief AL `PageVariable.RunModal()`: this variable's own page object runs modally, through
  ///        the test's ModalPageHandler, and `GetRecord` afterwards reads what it stands on.
  /// \return The action it closed with.
  ::agiru::Action RunModal()
    requires(!std::is_void_v<Derived>)
  {
    return detail::RunInstance(static_cast<Derived &>(*this), true);
  }

  /// \brief AL `Page.RunModal(PageId [, Record])` -- shows the page and waits for it.
  /// \tparam Arguments What AL handed it.
  /// \param arguments The record, when there is one.
  /// \return The action the user closed it with.
  /// \throws Error until the UI runs (board:0030).
  /// \note IT RETURNS AN `Action` AND NOT AN `Integer`. Every `RunModal` page names
  ///       `Type: Action` as its return, and AL compares it against `Action::LookupOK` --
  ///       `if Page.RunModal(...) = Action::LookupOK then` is how the whole BaseApp reads a
  ///       lookup. An `Integer` there is not comparable to an `Action` and never was.
  /// \warning A CONTROL HAS ITS OWN TRIGGERS AND THEY ARE THE FIELD'S, NOT THE PAGE'S: a page
  ///          field runs `OnLookup`, `OnDrillDown`, `OnAssistEdit` and `OnControlAddIn`, and around
  ///          a write `OnBeforeValidate` then `OnAfterValidate`; a `pagefieldextension` adds
  ///          `OnAfterAfterLookup` beside them, an `actionextension` `OnBeforeAction` and
  ///          `OnAfterAction`. A table field's own `OnLookup` is the one AL declares in the TABLE
  ///          and the page's control inherits. Each is one page under `triggers-auto/`.

  /// \warning THE PLATFORM EVENTS FIRE WHETHER OR NOT THE PAGE DECLARES THE TRIGGER, which is why
  ///          they are the RUNTIME's and never the generated object's: `OnOpenPageEvent`,
  ///          `OnClosePageEvent`, `OnQueryClosePageEvent`, `OnAfterGetRecordEvent`,
  ///          `OnAfterGetCurrRecordEvent`, `OnNewRecordEvent`, `OnInsertRecordEvent`,
  ///          `OnModifyRecordEvent`, `OnDeleteRecordEvent`, and around an action
  ///          `OnBeforeActionEvent` then `OnAfterActionEvent`. A page that declares none of them
  ///          still raises all of them, the way `Table::Insert` raises `OnBeforeInsertEvent`
  ///          without asking the table.
  ///
  /// \warning THE TRIGGER ORDER IS OWED AND NAMED HERE, because a page that runs must run them in
  ///          the platform's order and not in the generator's: `OnInit`, then `OnOpenPage`, then
  ///          per record `OnFindRecord`, `OnAfterGetRecord` and `OnAfterGetCurrRecord`; a write
  ///          runs `OnInsertRecord`, `OnModifyRecord` or `OnDeleteRecord`; the close runs
  ///          `OnQueryClosePage` and then `OnClosePage`. Each is one page under `triggers-auto/`,
  ///          and naming them here is what keeps them from being a silent hole while the UI is
  ///          board:0030's work -- nothing fires until there is a page to fire it on.
  template <typename... Arguments>
    requires(sizeof...(Arguments) > 0)
  static ::agiru::Action RunModal(Arguments &&...arguments) {
    if constexpr (std::is_void_v<Derived>) {
      return detail::RunPageByNumber(true, arguments...);
    } else {
      return detail::RunPage<Derived>(true, arguments...);
    }
  }

  /// \brief AL `Page.Activate(Boolean)`. Activates the current page on the client if possible. The
  /// data on the page will not be refreshed.
  /// \param Refresh The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  /// \brief AL `Page.Activate()` -- the READING form, which the documentation brackets:
  ///        `[X := ] Page.Activate([NewX])`.
  /// \return Never.
  /// \throws Error always -- a page property needs a running UI (board:0030).
  [[nodiscard]] ::agiru::Boolean Activate() const {
    throw Error("Page.Activate() needs a running UI (board:0030)");
  }

  ::agiru::Boolean Activate(::agiru::Boolean Refresh) {
    static_cast<void>(Refresh);
    throw Error("Page.Activate(Boolean) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.CancelBackgroundTask(Integer)`. Attempt to cancel a page background task.
  /// \param TaskId The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Boolean CancelBackgroundTask(::agiru::Integer TaskId) {
    static_cast<void>(TaskId);
    throw Error("Page.CancelBackgroundTask(Integer) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.Caption()` -- the READING form, which the documentation's syntax block
  /// brackets: `[X := ] Page.Caption([NewCaption])`.
  /// \return The caption the page shows.
  /// \throws Error until the UI runs (board:0030).
  std::string Caption() const {
    if (!caption_.empty()) { return caption_; }
    if constexpr (!std::is_void_v<Derived>) {
      return std::string(PageTraits<Derived>::kPage.caption);
    } else {
      return {};
    }
  }

  /// \brief AL `Page.Caption(Text)`. The caption shown in the title bar. For example, the default
  /// value in English (United States) is the same as the name of the page.
  /// \param NewCaption The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error until the UI runs (board:0030).
  std::string Caption(std::string_view NewCaption) {
    caption_ = std::string(NewCaption);
    return caption_;
  }

  /// \brief AL `Page.Close()`. Closes the current page.
  /// \throws Error until the UI runs (board:0030).
  void Close() { throw Error("Page.Close() needs a running UI (board:0030)"); }

  /// \brief AL `Page.Editable(Boolean)`. Gets or sets the default editability of the page.
  /// \param NewEditable The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  /// \brief AL `Page.Editable()` -- the READING form, which the documentation brackets:
  ///        `[X := ] Page.Editable([NewX])`.
  /// \return Never.
  /// \throws Error always -- a page property needs a running UI (board:0030).
  [[nodiscard]] ::agiru::Boolean Editable() const { return editable_; }

  ::agiru::Boolean Editable(::agiru::Boolean NewEditable) {
    const ::agiru::Boolean was = editable_;
    editable_ = NewEditable;
    return was;
  }

  /// \brief AL `Page.EnqueueBackgroundTask(Integer, Integer, Dictionary of [Text, Text], Integer,
  /// PageBackgroundTaskErrorLevel)`. Creates and queues a background task that runs the specified
  /// codeunit (without a UI) in a read-only child session of the page session. If the task
  /// completes successfully, the **OnPageBackgroundTaskCompleted** trigger is invoked. If an error
  /// occurs, the **OnPageBackgroundTaskError** trigger is invoked. If the page is closed before the
  /// task completes, or the page record ID on the task changed, the task is cancelled.
  /// \param TaskId The AL `Integer`.
  /// \param CodeunitId The AL `Integer`.
  /// \param Parameters The AL `Dictionary of [Text, Text]`.
  /// \param Timeout The AL `Integer`.
  /// \param ErrorLevel The AL `PageBackgroundTaskErrorLevel`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Boolean
  EnqueueBackgroundTask(::agiru::Integer &TaskId,
                        ::agiru::Integer CodeunitId,
                        ::agiru::Dictionary<::agiru::Text<0>, ::agiru::Text<0>> &Parameters,
                        ::agiru::Integer Timeout = {},
                        const ::agiru::PageBackgroundTaskErrorLevel &ErrorLevel =
                            ::agiru::PageBackgroundTaskErrorLevel{}) {
    static_cast<void>(TaskId);
    static_cast<void>(CodeunitId);
    static_cast<void>(Parameters);
    static_cast<void>(Timeout);
    static_cast<void>(ErrorLevel);
    throw Error("Page.EnqueueBackgroundTask(Integer, Integer, Dictionary of [Text, Text], Integer, "
                "PageBackgroundTaskErrorLevel) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.GetBackgroundParameters()`. Gets the page background task input parameters.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Dictionary<::agiru::Text<0>, ::agiru::Text<0>> GetBackgroundParameters() {
    throw Error("Page.GetBackgroundParameters() needs a running UI (board:0030)");
  }

  /// \brief AL `Page.GetRecord(Record)`. Gets the current record of the page.
  /// \tparam Record The table the caller hands over.
  /// \param record The record.
  /// \throws Error until the UI runs (board:0030).
  /// \note A BARE `Record` PARAMETER TAKES ANY TABLE, which is what makes it a template here.
  ///       `RecordRef` is a different AL type -- a record reached by NUMBER -- and using it
  ///       would refuse every call that hands over a record it has.
  template <typename Record> void GetRecord(Record &record) {
    if constexpr (requires { record.operator->(); }) {
      GetRecord(*record.operator->());
    } else if constexpr (requires { record = static_cast<Derived &>(*this).Rec; }) {
      record = static_cast<Derived &>(*this).Rec;
    } else {
      static_cast<void>(record);
      throw Error("Page.GetRecord(Record): the record is not of the page's source table");
    }
  }

  /// \brief AL `Page.LookupMode()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] Page.LookupMode([NewX])`.
  /// \return The value it holds.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Boolean LookupMode() const { return lookupMode_; }

  /// \brief AL `Page.LookupMode(Boolean)`. Gets or sets the default lookup mode for the page.
  /// \param NewLookupMode The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Boolean LookupMode(::agiru::Boolean NewLookupMode) {
    lookupMode_ = NewLookupMode;
    return lookupMode_;
  }

  /// \brief AL `Page.ObjectId()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] Page.ObjectId([NewX])`.
  /// \return The value it holds.
  /// \throws Error until the UI runs (board:0030).
  [[nodiscard]] std::string ObjectId() const {
    throw Error("Page.ObjectId() needs a running UI (board:0030)");
  }

  /// \brief AL `Page.ObjectId(Boolean)`. Returns a string in the "Page xxx" format, where xxx is
  /// the caption or ID of the application object.
  /// \param UseNames The AL `Boolean`.
  /// \return The AL `Text`.
  /// \throws Error until the UI runs (board:0030).
  std::string ObjectId(::agiru::Boolean UseNames) {
    static_cast<void>(UseNames);
    throw Error("Page.ObjectId(Boolean) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.PromptMode()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] Page.PromptMode([NewX])`.
  /// \return The value it holds.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::PromptMode PromptMode() const {
    throw Error("Page.PromptMode() needs a running UI (board:0030)");
  }

  /// \brief AL `Page.PromptMode(PromptMode)`. The mode of a PromptDialog page that prompts the user
  /// for input and shows the output of a copilot interaction.
  /// \param NewPromptMode The AL `PromptMode`.
  /// \return The AL `PromptMode`.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::PromptMode PromptMode(const ::agiru::PromptMode &NewPromptMode) {
    static_cast<void>(NewPromptMode);
    throw Error("Page.PromptMode(PromptMode) needs a running UI (board:0030)");
  }

  /// \note A PAGE ON NO RECORD SAVES NOTHING. `page-saverecord-method.md` says "if the record does
  ///       not exist it is inserted, otherwise it is modified" -- of the CURRENT record, and an
  ///       empty list has none: `Price Worksheet` calls `CurrPage.SaveRecord()` from its
  ///       `OnOpenPage`, and inserting the blank row there put a `("", 1)` worksheet line in the
  ///       way of the copy that followed (`Suggest Price Lines UT`, 12 cases; board:0705).
  /// \brief AL `Page.SaveRecord()`. Saves the current record as if performed by the client. If the
  /// record does not exist it is inserted, otherwise it is modified.
  /// \throws Error until the UI runs (board:0030).
  void SaveRecord() {
    if constexpr (requires {
                    static_cast<Derived &>(*this).Rec.Insert(true);
                    typename std::remove_cvref_t<
                        decltype(static_cast<Derived &>(*this).Rec)>::Platform_Half;
                  }) {
      auto &rec = static_cast<Derived &>(*this).Rec;
      using Source = std::remove_cvref_t<decltype(rec)>;
      Source probe = rec;
      if (static_cast<typename Source::Platform_Half &>(probe).Find("=")) {
        rec.Modify(true);
        return;
      }
      if (!newRecord_) { return; }
      rec.Insert(true);
      newRecord_ = false;
    } else {
      throw Error("Page.SaveRecord(): the page has no source table");
    }
  }

  /// \brief The runner says the page stands on a NEW record -- `OnNewRecord` is about to run --
  ///        which is the one `SaveRecord` inserts.
  void StartedNewRecord() { newRecord_ = true; }

  /// \brief The runner says the page landed on an existing record.
  void LandedOnRecord() { newRecord_ = false; }

  /// \brief Whether the page still stands on a record nothing has inserted: false once
  ///        `SaveRecord` wrote it, so the test harness does not write it a second time
  ///        (`Price List Line UT`, a line inserted by `CurrPage.SaveRecord()` and again on leaving
  ///        the row, 2026-09-12).
  /// \return True while the record is new.
  [[nodiscard]] bool StandsOnNewRecord() const { return newRecord_; }

  /// \brief AL `Page.SetBackgroundTaskResult(Dictionary of [Text, Text])`. Sets the page background
  /// task result as a dictionary. When the task is completed, the OnPageBackgroundCompleted trigger
  /// will be invoked on the page with this result dictionary.
  /// \param Results The AL `Dictionary of [Text, Text]`.
  /// \throws Error until the UI runs (board:0030).
  void
  SetBackgroundTaskResult(const ::agiru::Dictionary<::agiru::Text<0>, ::agiru::Text<0>> &Results) {
    static_cast<void>(Results);
    throw Error(
        "Page.SetBackgroundTaskResult(Dictionary of [Text, Text]) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.SetRecord(Record)`. Sets the current record for the page.
  /// \tparam Record The table the caller hands over.
  /// \param record The record.
  /// \throws Error until the UI runs (board:0030).
  /// \note A BARE `Record` PARAMETER TAKES ANY TABLE, which is what makes it a template here.
  ///       `RecordRef` is a different AL type -- a record reached by NUMBER -- and using it
  ///       would refuse every call that hands over a record it has.
  template <typename Record> void SetRecord(Record &record) {
    if constexpr (requires { static_cast<Derived &>(*this).Rec = record; }) {
      static_cast<Derived &>(*this).Rec = record;
    } else {
      static_cast<void>(record);
      throw Error("Page.SetRecord(Record): the record is not of the page's source table");
    }
  }

  /// \brief AL `Page.SetSelectionFilter(Record)`. Notes the records that the user has selected on
  /// the page, marks those records in the table specified, and sets the filter to "marked only".
  /// \tparam Record The table the caller hands over.
  /// \param record The record.
  /// \throws Error until the UI runs (board:0030).
  /// \note A BARE `Record` PARAMETER TAKES ANY TABLE, which is what makes it a template here.
  ///       `RecordRef` is a different AL type -- a record reached by NUMBER -- and using it
  ///       would refuse every call that hands over a record it has.
  template <typename Record> void SetSelectionFilter(Record &record) {
    if constexpr (requires { record.operator->(); }) {
      SetSelectionFilter(*record.operator->());
    } else if constexpr (requires {
                           record = static_cast<Derived &>(*this).Rec;
                           record.SetRecFilter();
                         }) {
      record = static_cast<Derived &>(*this).Rec;
      record.SetRecFilter();
    } else {
      static_cast<void>(record);
      throw Error("Page.SetSelectionFilter(Record): the record is not of the page's source table");
    }
  }

  /// \brief AL `Page.SetTableView(Record)`. Applies the table view on the current record as the
  /// table view for the page, report, or XmlPort.
  /// \tparam Record The table the caller hands over.
  /// \param record The record.
  /// \throws Error until the UI runs (board:0030).
  /// \note A BARE `Record` PARAMETER TAKES ANY TABLE, which is what makes it a template here.
  ///       `RecordRef` is a different AL type -- a record reached by NUMBER -- and using it
  ///       would refuse every call that hands over a record it has.
  template <typename Record> void SetTableView(Record &record) {
    if constexpr (requires { record.operator->(); }) {
      SetTableView(*record.operator->());
    } else if constexpr (requires { static_cast<Derived &>(*this).Rec.CopyFilters(record); }) {
      static_cast<Derived &>(*this).Rec.CopyFilters(record);
    } else {
      static_cast<void>(record);
      throw Error("Page.SetTableView(Record): the record is not of the page's source table");
    }
  }

  /// \brief AL `Page.Update(Boolean)`. Saves the current record and then updates the controls on
  /// the page. If you set the SaveRecord parameter to false, this method will not save the record
  /// before the page is updated.
  /// \param SaveRecord The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  /// \brief AL `Page.Update([SaveRecord])`: "Saves the current record and then updates the
  ///        controls on the page" (`page-update-method.md`).
  /// \param SaveRecord Whether the current record is saved first.
  ///
  /// \note HEADLESS, THE CONTROLS ARE THE RECORD, so refreshing them is nothing to do; and the
  ///       SAVE is the harness's row-leave, which board:0030 still owes -- until it exists,
  ///       `SaveRecord` is carried and acted on by nothing rather than refused (15 cases stopped
  ///       here on 2026-09-09, each in a page trigger the test drove through a `TestPage`).
  void Update(::agiru::Boolean SaveRecord = true) { static_cast<void>(SaveRecord); }

  /// \note NO PROTECTED DESTRUCTOR AND NO PRIVATE CONSTRUCTOR, for the reason `Table` gives: a
  ///       generated class has no user-declared constructor, so `pages::X P{}` is aggregate
  ///       initialisation and both of those make it fail from the caller's context.

private:
  bool editable_ = true;
  bool modal_ = false;
  bool newRecord_ = false;
  ::agiru::Action closeAction_ = ::agiru::Action::OK;
  std::string caption_;
  ::agiru::Boolean lookupMode_ = false;
};

/// \brief AL `Page.Run(Number, ...)` and `Page.RunModal(Number, ...)` by object NUMBER, the way
///        `Codeunit<void>` carries `Codeunit.Run(Number)`: refused until the page catalogue exists
///        (board:0038).
template <> class Page<void> {
public:
  /// \brief AL `PAGE.GetBackgroundParameters(...)` -- the parameters a page background task was
  /// started with.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- there is no running page yet (board:0030).
  template <typename... Arguments>
  static ::agiru::Dictionary<::agiru::Text<0>, ::agiru::Text<0>>
  GetBackgroundParameters(Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Page.GetBackgroundParameters is declared and needs a running UI (board:0030)");
  }

  /// \brief AL `PAGE.SetFilterToMultipleValues(...)` -- a filter naming several values at once.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- there is no running page yet (board:0030).
  /// \brief AL `PAGE.SetBackgroundTaskResult(Dictionary)` -- what a page background task hands
  ///        back to its page.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- there is no background task without a running page (board:0030).
  template <typename... Arguments> static void SetBackgroundTaskResult(Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Page.SetBackgroundTaskResult is declared and needs a running UI (board:0030)");
  }

  template <typename... Arguments> static void SetFilterToMultipleValues(Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Page.SetFilterToMultipleValues is declared and needs a running UI (board:0030)");
  }

  template <typename... Arguments>
  static void Run(::agiru::Integer Number, Arguments &&...arguments) {
    static_cast<void>(detail::RunPageByNumber(false, Number, arguments...));
  }

  template <typename... Arguments>
  static ::agiru::Action RunModal(::agiru::Integer Number, Arguments &&...arguments) {
    return detail::RunPageByNumber(true, Number, arguments...);
  }
};

}
