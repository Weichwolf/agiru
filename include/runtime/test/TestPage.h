#pragma once

#include "meta/PageDef.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/Record.h"
#include "runtime/Table.h"
#include "runtime/test/PageCore.h"
#include "runtime/test/TestAction.h"
#include "runtime/test/TestField.h"
#include "runtime/test/TestFilter.h"
#include "type/Boolean.h"
#include "type/Dictionary.h"
#include "type/Integer.h"
#include "type/Text.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

/// \file
/// \brief AL `TestPage` -- a page driven without a screen.

namespace agiru {

/// \brief AL `TestPage <Page>` -- the page, headless, with its controls reachable by name.
///
/// \tparam P The generated page class.
///
/// \note IT DERIVES FROM THE PAGE, and that is what makes `SalesOrder."No.".SetValue('X')` work at
///       all. AL reaches a control THROUGH the TestPage, so the controls have to be members of it;
///       the page already declares every one of them as a `TestField`, and inheriting is how C++
///       says "and all of those too" without the generator writing them a second time.
///
/// \warning A CONTROL WHOSE AL NAME IS ONE OF THE METHODS BELOW IS HIDDEN BY IT. C++ resolves the
///          derived name first, so a page with an action called `Close` would find this `Close`.
///          The failure is loud -- a wrong signature at the call site -- rather than silent, and no
///          page in the read roots has one today.
/// \brief A page this run never translated, so the controls are not there to reach.
///
/// \note IT IS NOT AN EMPTY PAGE. A `TestPage` over a page the transpiler has not seen must refuse
///       every control by NAME rather than silently having none, so what stands here is a class
///       with no members at all: the call site fails to compile and says which control it wanted.
class UnknownPage {};

}

/// \brief The controls a `TestPage` over an untranslated page has, which is none.
template <> struct agiru::PageTraits<agiru::UnknownPage> {
  /// \brief No controls at all.
  /// \tparam Field_Kind  How a test reaches a control.
  /// \tparam Action_Kind How a test reaches an action.
  /// \tparam Part_Kind   How a test reaches an embedded page.
  ///
  /// \note THE INTERIOR UNDERSCORE IS THE SEAM. `Field`, `Action` and `Part` are names a page
  ///       control may carry, and a control of that name shadows the template parameter -- 40
  ///       headers failed on `declaration of 'Field' shadows template parameter`. No AL name
  ///       reaches an interior underscore.
  template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>
  using Controls = agiru::UnknownPage;
};

namespace agiru {

/// \brief AL `TestPage <Page>` -- the page, headless, with its controls reachable by name.
/// \tparam P The generated page class.
/// \note A `part` IS A NESTED `TestPage`, which is why `TestPage` names ITSELF as the third
///       argument. AL reaches through an embedded page -- `SalesInvoice.SalesLines.Description.
///       SetValue(...)` -- so the part has to carry the subform's own controls, and a `TestField`,
///       which has a value and no controls, cannot.
template <typename P = UnknownPage>
class TestPage : public PageTraits<P>::template Controls<TestField, TestAction, ::agiru::TestPage>,
                 public PageCore {
public:
  /// \brief Marks the type for the handler thunk, which binds a `[PageHandler]`'s parameter.
  using IsTestPage = void;

  /// \brief Whether the page has a source table; a dialog with none has no rows to move over.
  static constexpr bool kHasRecord =
      requires(P &page) { page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{}); };

  TestPage() = default;

  /// \brief A copy is a second HANDLE on the same page, as an AL `TestPage` passed by value is.
  /// \param o The harness copied.
  TestPage(const TestPage &o) { Share_(o); }

  /// \brief The same, by assignment.
  /// \param o The harness copied.
  /// \return This.
  TestPage &operator=(const TestPage &o) {
    if (this != &o) {
      Release_();
      Share_(o);
    }
    return *this;
  }

  /// \brief A move takes the page over, ownership and all; `Clear(TestPage)` is one.
  /// \param o The harness moved from, which holds nothing afterwards.
  TestPage(TestPage &&o) noexcept { Take_(o); }

  /// \brief The same, by assignment.
  /// \param o The harness moved from.
  /// \return This.
  TestPage &operator=(TestPage &&o) noexcept {
    if (this != &o) {
      Release_();
      Take_(o);
    }
    return *this;
  }

  ~TestPage() override { Release_(); }

  /// \brief AL `TestPage.OpenNew()` -- opens the page on a new record.
  /// \throws Error when the page is already open, as AL does.
  void OpenNew() { Open_(true, true); }

  /// \brief AL `TestPage.OpenEdit()` -- opens the page on its first record, for editing.
  /// \throws Error when the page is already open, as AL does.
  void OpenEdit() { Open_(true, false); }

  /// \brief AL `TestPage.OpenView()` -- opens the page on its first record, read-only.
  /// \throws Error when the page is already open, as AL does.
  void OpenView() { Open_(false, false); }

  /// \brief AL `TestPage.Close()` -- runs `OnQueryClosePage` and `OnClosePage`, then lets go.
  void Close() {
    if (page_ == nullptr) { return; }
    detail::ClosePage(*page_);
    Release_();
  }

  /// \brief AL `TestPage.First()`.
  /// \return Whether there is a first record.
  Boolean First() {
    return Landed_([](auto &rec) { return static_cast<bool>(Platform_(rec).FindFirst()); });
  }

  /// \brief AL `TestPage.Next()`.
  /// \return Whether there is a next record.
  Boolean Next() {
    return Landed_([](auto &rec) { return Platform_(rec).Next() != 0; });
  }

  /// \brief AL `TestPage.Previous()`.
  /// \return Whether there is a previous record.
  Boolean Previous() {
    return Landed_([](auto &rec) { return Platform_(rec).Next(-1) != 0; });
  }

  /// \brief AL `TestPage.Prev()`, the older spelling of `Previous`.
  /// \return Whether there is a previous record.
  Boolean Prev() { return Previous(); }

  /// \brief AL `TestPage.Last()`.
  /// \return Whether there is a last record.
  Boolean Last() {
    return Landed_([](auto &rec) { return static_cast<bool>(Platform_(rec).FindLast()); });
  }

  /// \brief AL `TestPage.New()` -- moves to a new record and runs `OnNewRecord`.
  void New() {
    if constexpr (kHasRecord) {
      static_cast<void>(Page_());
      Relink_();
      Platform_(Record_()).Init();
      if constexpr (requires { Page_().OnNewRecord(Boolean{}); }) { Page_().OnNewRecord(false); }
      detail::AfterGetRecord(Page_());
    } else {
      Unopened_();
    }
  }

  /// \brief AL `TestPage.GoToRecord(Record)` -- positions the page on that record.
  /// \tparam R The record's type.
  /// \param Record The record.
  /// \return Whether it was found within the page's filters.
  template <typename R> Boolean GoToRecord(const R &Record) {
    if constexpr (requires { Record.RecordId(); }) {
      return Landed_([&](auto &rec) { return Platform_(rec).Get(Record.RecordId()); });
    } else {
      return Landed_(
          [&](auto &rec) { return Platform_(rec).Get(detail::RecordIdInVariant(Record)); });
    }
  }

  /// \brief AL `TestPage.GoToKey(Values, ...)` -- positions the page on the record with that key.
  /// \tparam Values The key values' types.
  /// \param values The key values, in key order.
  /// \return Whether it was found.
  template <typename... Values> Boolean GoToKey(const Values &...values) {
    return Landed_([&](auto &rec) { return Platform_(rec).Get(values...); });
  }

  /// \brief AL `TestPage.Trap()` -- the next non-modal run of this page lands here.
  /// \return This.
  TestPage &Trap() {
    if constexpr (requires { PageTraits<P>::kId; }) {
      detail::TrapPage(PageTraits<P>::kId.Value(), this, &TestPage::AdoptOwned_);
      return *this;
    } else {
      Unopened_();
    }
  }

  /// \brief Takes a page another runner opened, without owning it; a `[PageHandler]` gets one.
  /// \param page The page object, opened already.
  void Adopt(void *page) {
    Release_();
    page_ = static_cast<P *>(page);
    owned_ = false;
    Bind_();
  }

  /// \brief AL `TestPage.OK()` -- the page's OK action.
  /// \return The action, to `Invoke`.
  TestAction OK() { return Bound_("OK"); }

  /// \brief AL `TestPage.Cancel()` -- the page's Cancel action.
  /// \return The action, to `Invoke`.
  TestAction Cancel() { return Bound_("Cancel"); }

  /// \brief AL `TestPage.Yes()` -- a confirmation's Yes.
  /// \return The action, to `Invoke`.
  TestAction Yes() { return Bound_("Yes"); }

  /// \brief AL `TestPage.No()` -- a confirmation's No.
  /// \return The action, to `Invoke`.
  TestAction No() { return Bound_("No"); }

  /// \brief AL `TestPage.Edit()` -- switches a view-mode page to editing.
  /// \return The action, to `Invoke`.
  TestAction Edit() { return Bound_("Edit"); }

  /// \brief AL `TestPage.Caption()`.
  /// \return The page's caption, or its name.
  [[nodiscard]] Text<0> Caption() const {
    if constexpr (requires { PageTraits<P>::kPage; }) {
      return Text<0>{PageTraits<P>::kPage.caption.empty() ? PageTraits<P>::kName
                                                          : PageTraits<P>::kPage.caption};
    } else {
      Unopened_();
    }
  }

  /// \brief AL `TestPage.Editable()`.
  /// \return Whether the page was opened for editing.
  [[nodiscard]] Boolean Editable() const {
    if constexpr (requires(P &page) { page.OpenedEditable(); }) {
      return page_ != nullptr && page_->OpenedEditable();
    } else {
      return false;
    }
  }

  /// \brief AL `TestPage.Expand(Boolean)` -- expands or collapses the current row of a tree.
  /// \param Expand True to expand.
  void Expand(Boolean Expand) { static_cast<void>(Expand); }

  /// \brief AL `TestPage.IsExpanded()`.
  /// \return False; rows do not nest here yet.
  [[nodiscard]] Boolean IsExpanded() const { return false; }

  /// \brief AL `TestPage.GetValidationError()`.
  /// \return The last validation error's text; empty, because a validation error THROWS here.
  [[nodiscard]] Text<0> GetValidationError() const { return Text<0>{}; }

  /// \brief AL `TestPage.ValidationErrorCount()`.
  /// \return Zero, for the same reason.
  [[nodiscard]] Integer ValidationErrorCount() const { return 0; }

  /// \brief AL `TestPage.FindFirstField(Field, Value)` -- the first row whose control shows it.
  /// \tparam V The value's type.
  /// \param field The control.
  /// \param value What it must show.
  /// \return Whether a row does.
  template <typename V> Boolean FindFirstField(const TestField &field, const V &value) {
    if (!First()) { return false; }
    return Matches_(field, value) || FindNextField(field, value);
  }

  /// \brief AL `TestPage.FindNextField(Field, Value)`.
  /// \tparam V The value's type.
  /// \param field The control.
  /// \param value What it must show.
  /// \return Whether a later row does.
  template <typename V> Boolean FindNextField(const TestField &field, const V &value) {
    while (Next()) {
      if (Matches_(field, value)) { return true; }
    }
    return false;
  }

  /// \brief AL `TestPage.FindPreviousField(Field, Value)`.
  /// \tparam V The value's type.
  /// \param field The control.
  /// \param value What it must show.
  /// \return Whether an earlier row does.
  template <typename V> Boolean FindPreviousField(const TestField &field, const V &value) {
    while (Previous()) {
      if (Matches_(field, value)) { return true; }
    }
    return false;
  }

  /// \brief AL `TestPage.GetField(No)` -- a control by its field number.
  /// \param No The field number.
  /// \return The control, bound.
  TestField GetField(Integer No) {
    const ControlDef *control = ControlByField_(::agiru::FieldNo{No});
    if (control == nullptr) {
      throw Error("no control on the page shows field " + std::to_string(No));
    }
    TestField field{control->name};
    field.Bind(*this);
    return field;
  }

  /// \brief AL `TestPage.RunPageBackgroundTask(...)`.
  /// \param CodeunitId            The codeunit.
  /// \param Parameters            Its parameters.
  /// \param RunCompletionTriggers Whether to run the completion triggers.
  /// \return Never.
  /// \throws Error always -- background tasks have no runner yet (board:0030).
  Dictionary<::agiru::Text<0>, ::agiru::Text<0>>
  RunPageBackgroundTask(Integer CodeunitId,
                        Dictionary<::agiru::Text<0>, ::agiru::Text<0>> &Parameters,
                        Boolean RunCompletionTriggers = {}) {
    static_cast<void>(CodeunitId);
    static_cast<void>(Parameters);
    static_cast<void>(RunCompletionTriggers);
    Unopened_();
  }

  /// \brief AL `TestPage.Filter` -- the page's filter pane, bound when the page opens.
  TestFilter Filter{}; // NOLINT(misc-non-private-member-variables-in-classes)

  void SetControlText(std::string_view control, std::string_view text) override {
    static_cast<void>(Page_());
    Relink_();
    const ControlDef *def = ControlNamed_(control);
    if (def == nullptr || def->field.Value() == 0) {
      throw Error("the control '" + std::string(control) + "' shows no field to set");
    }
    if constexpr (kHasRecord) {
      try {
        auto before = Record_();
        PageValidateEvent_("OnBeforeValidateEvent", def->name, before);
        Record_().ValidateText(def->field, text);
        if (text.empty() && Record_().FieldNotBlank(def->field)) {
          Record_().TestField(def->field);
        }
        RunTrigger_(control, ControlTriggerKind::Validate, true);
        PageValidateEvent_("OnAfterValidateEvent", def->name, before);
      } catch (const Error &e) { throw e.Coded("TestValidation"); }
    } else {
      static_cast<void>(text);
      Unopened_();
    }
  }

  [[nodiscard]] std::string ControlText(std::string_view control) const override {
    const ControlDef *def = ControlNamed_(control);
    if (def == nullptr || def->field.Value() == 0) {
      throw Error("the control '" + std::string(control) + "' shows no field to read");
    }
    if constexpr (kHasRecord) {
      return Record_().FieldFormat(def->field);
    } else {
      Unopened_();
    }
  }

  void RunControlTrigger(std::string_view control, ControlTriggerKind kind) override {
    if (kind == ControlTriggerKind::Action && CloseAction_(control)) { return; }
    RunTrigger_(control, kind, kind == ControlTriggerKind::Action);
  }

  void SetControlFilter(std::string_view control, std::string_view filter) override {
    if constexpr (kHasRecord) {
      const ControlDef *def = ControlNamed_(control);
      ::agiru::FieldNo no = def != nullptr ? def->field : ::agiru::FieldNo{};
      if (no.Value() == 0) {
        for (const FieldDef &field : RecordTraits_().kTable.fields) {
          if (SameWord_(field.name, control)) { no = field.no; }
        }
      }
      if (no.Value() == 0) {
        throw Error("the control '" + std::string(control) + "' shows no field to filter");
      }
      Record_().SetFilterOn(no, filter);
      static_cast<void>(First());
    } else {
      static_cast<void>(filter);
      Unopened_();
    }
  }

  [[nodiscard]] Boolean ControlVisible(std::string_view control) const override {
    if (const auto computed = Computed_(control, &ControlTrigger<P>::visible); computed) {
      return *computed;
    }
    const ControlDef *def = ControlNamed_(control);
    return def == nullptr || !SameWord_(def->visible, "false");
  }

  [[nodiscard]] Boolean ControlEditable(std::string_view control) const override {
    if (const auto computed = Computed_(control, &ControlTrigger<P>::editable); computed) {
      return Editable() && *computed;
    }
    const ControlDef *def = ControlNamed_(control);
    return Editable() && (def == nullptr || !SameWord_(def->editable, "false"));
  }

  [[nodiscard]] Boolean ControlEnabled(std::string_view control) const override {
    if (const auto computed = Computed_(control, &ControlTrigger<P>::enabled); computed) {
      return *computed;
    }
    const ControlDef *def = ControlNamed_(control);
    return def == nullptr || !SameWord_(def->enabled, "false");
  }

  [[nodiscard]] std::optional<Boolean>
  Computed_(std::string_view control, ::agiru::Boolean (P::*ControlTrigger<P>::*state)()) const {
    if constexpr (requires { PageTraits<P>::kControlTriggers; }) {
      if (page_ == nullptr) { return std::nullopt; }
      for (const ControlTrigger<P> &trigger : PageTraits<P>::kControlTriggers) {
        if (!SameWord_(trigger.control, control)) { continue; }
        ::agiru::Boolean (P::*compute)() = trigger.*state;
        if (compute == nullptr) { return std::nullopt; }
        return (page_->*compute)();
      }
    } else {
      static_cast<void>(control);
      static_cast<void>(state);
    }
    return std::nullopt;
  }

  /// \brief AL `Parent.Part` bound as a nested test page: attaches to the parent's subpage on
  ///        first use and follows its `SubPageLink` from then on.
  /// \param parent The parent test page.
  /// \param name The part control's AL name.
  void BindPart(PageCore &parent, std::string_view name) {
    parent_ = &parent;
    partName_ = std::string(name);
    page_ = nullptr;
    owned_ = false;
    Bind_();
  }

  [[nodiscard]] void *PartInstance(std::string_view control) override {
    if constexpr (requires(P &page) { page.PartInstance(control); }) {
      return page_ == nullptr ? nullptr : page_->PartInstance(control);
    } else {
      return nullptr;
    }
  }

  void LinkPart(std::string_view control, void *subRecord, const TableDef &subTable) override {
    if constexpr (kHasRecord) {
      const ControlDef *def = ControlNamed_(control);
      if (def == nullptr || def->subPageLink.empty() || page_ == nullptr) { return; }
      detail::ApplySubPageLink(subRecord,
                               subTable,
                               static_cast<const void *>(&page_->Rec),
                               RecordTraits_().kTable,
                               def->subPageLink);
    } else {
      static_cast<void>(control);
      static_cast<void>(subRecord);
      static_cast<void>(subTable);
    }
  }

  [[nodiscard]] std::string ControlCaption(std::string_view control) const override {
    const ControlDef *def = ControlNamed_(control);
    if (def == nullptr) { return std::string(control); }
    if (!def->caption.empty()) { return std::string(def->caption); }
    if constexpr (kHasRecord) {
      if (def->field.Value() != 0) {
        return std::string(::agiru::FieldCaption(RecordTraits_().kTable, def->field));
      }
    }
    return std::string(def->name);
  }

private:
  [[noreturn]] static void Unopened_() {
    throw Error("a TestPage needs a running page (board:0030)");
  }

  template <typename R> static typename std::remove_cvref_t<R>::Platform_Half &Platform_(R &rec) {
    return static_cast<typename std::remove_cvref_t<R>::Platform_Half &>(rec);
  }

  static bool SameWord_(std::string_view text, std::string_view word) {
    return text.size() == word.size() &&
           std::ranges::equal(text, word, [](unsigned char a, unsigned char b) {
             return std::tolower(a) == std::tolower(b);
           });
  }

  P &Page_() {
    if (page_ == nullptr && parent_ != nullptr) { Attach_(); }
    if (page_ == nullptr) { Unopened_(); }
    return *page_;
  }

  void Attach_() {
    if constexpr (requires { PageTraits<P>::kPage; }) {
      auto *sub = static_cast<P *>(parent_->PartInstance(partName_));
      if (sub == nullptr) { return; }
      page_ = sub;
      owned_ = false;
      Bind_();
      Relink_();
      detail::OpenPage(*page_, true, false);
    }
  }

  void Relink_() {
    if constexpr (kHasRecord) {
      if (parent_ != nullptr && page_ != nullptr) {
        parent_->LinkPart(partName_, static_cast<void *>(&page_->Rec), RecordTraits_().kTable);
      }
    }
  }

  const P &Page_() const {
    if (page_ == nullptr) { Unopened_(); }
    return *page_;
  }

  auto &Record_()
    requires kHasRecord
  {
    return Page_().Rec;
  }

  const auto &Record_() const
    requires kHasRecord
  {
    return Page_().Rec;
  }

  static auto RecordTraits_()
    requires kHasRecord
  {
    return TableTraits<std::remove_cvref_t<decltype(std::declval<P &>().Rec)>>{};
  }

  void Open_(bool editable, bool isNew) {
    if (page_ != nullptr) {
      throw Error("The TestPage is already open and cannot be opened again.");
    }
    if constexpr (requires { PageTraits<P>::kPage; }) {
      page_ = new P();
      owned_ = true;
      Bind_();
      detail::OpenPage(*page_, editable, isNew);
    } else {
      static_cast<void>(editable);
      static_cast<void>(isNew);
      Unopened_();
    }
  }

  void Take_(TestPage &o) {
    page_ = o.page_;
    owned_ = o.owned_;
    o.page_ = nullptr;
    o.owned_ = false;
    if (page_ != nullptr) { Bind_(); }
  }

  void Share_(const TestPage &o) {
    page_ = o.page_;
    owned_ = false;
    if (page_ != nullptr) { Bind_(); }
  }

  void Release_() {
    if (page_ != nullptr && owned_) { delete page_; }
    page_ = nullptr;
    owned_ = false;
  }

  void Bind_() {
    if constexpr (requires { this->BindControls(*static_cast<PageCore *>(this)); }) {
      this->BindControls(*static_cast<PageCore *>(this));
    }
    Filter.Bind(*this);
  }

  static void AdoptOwned_(void *harness, void *page) {
    auto &self = *static_cast<TestPage *>(harness);
    self.Release_();
    self.page_ = static_cast<P *>(page);
    self.owned_ = true;
    self.Bind_();
  }

  TestAction Bound_(std::string_view name) {
    TestAction action{name};
    action.Bind(*this);
    return action;
  }

  template <typename Step> Boolean Landed_(Step step) {
    if constexpr (kHasRecord) {
      static_cast<void>(Page_());
      Relink_();
      const bool found = step(Record_());
      if (found) { detail::AfterGetRecord(Page_()); }
      return found;
    } else {
      static_cast<void>(step);
      Unopened_();
    }
  }

  template <typename V> bool Matches_(const TestField &field, const V &value) {
    return ControlText(field.Name()) == AsText(value);
  }

  /// `OnBeforeValidateEvent` and `OnAfterValidateEvent` EXIST FOR PAGES (`devenv-event-types.md`
  /// lists the field events for pages as for tables): the topic is the PAGE's name and the element
  /// the control, and a subscriber hangs on exactly one of the two. Only the table's form was
  /// raised, so subscribers on 14 pages never ran (openerp WI-1402, board:0636).
  template <typename Before>
  void PageValidateEvent_(std::string_view event, std::string_view control, Before &before) {
    static constexpr std::array<std::string_view, 2> kNames{"Rec", "xRec"};
    auto &rec = Record_();
    ::agiru::detail::RaiseEventOn(EventObject::Page,
                                  PageTraits<P>::kId.Value(),
                                  PageTraits<P>::kName,
                                  event,
                                  control,
                                  kNames,
                                  rec,
                                  before);
  }

  bool CloseAction_(std::string_view control) {
    static constexpr std::array<std::pair<std::string_view, ::agiru::Action>, 5> kSystem{
        {{"OK", ::agiru::Action::OK},
         {"Cancel", ::agiru::Action::Cancel},
         {"Yes", ::agiru::Action::Yes},
         {"No", ::agiru::Action::No},
         {"LookupOK", ::agiru::Action::LookupOK}}};
    for (const auto &[name, action] : kSystem) {
      if (!SameWord_(control, name)) { continue; }
      if (ControlNamed_(control) != nullptr) { return false; }
      if constexpr (requires(P &page) { page.CloseWith(action); }) { Page_().CloseWith(action); }
      if (owned_) { Close(); }
      return true;
    }
    return false;
  }

  static void (P::*Trigger_(const ControlTrigger<P> &trigger, ControlTriggerKind kind))() {
    switch (kind) {
      case ControlTriggerKind::Validate: return trigger.validate;
      case ControlTriggerKind::Action: return trigger.action;
      case ControlTriggerKind::DrillDown: return trigger.drillDown;
      case ControlTriggerKind::AssistEdit: return trigger.assistEdit;
      case ControlTriggerKind::Lookup: return nullptr;
    }
    return nullptr;
  }

  void RunTrigger_(std::string_view control, ControlTriggerKind kind, bool optional) {
    if constexpr (requires { PageTraits<P>::kControlTriggers; }) {
      for (const ControlTrigger<P> &trigger : PageTraits<P>::kControlTriggers) {
        if (!SameWord_(trigger.control, control)) { continue; }
        if (kind == ControlTriggerKind::Lookup && trigger.lookup != nullptr) {
          ::agiru::Text<0> text(ControlText(control));
          if ((Page_().*trigger.lookup)(text)) { SetControlText(control, text.Value()); }
          return;
        }
        void (P::*run)() = Trigger_(trigger, kind);
        if (run != nullptr) {
          (Page_().*run)();
          return;
        }
        break;
      }
    }
    if (!optional) {
      throw Error("the control '" + std::string(control) + "' declares no such trigger");
    }
  }

  static const ControlDef *Within_(std::span<const ControlDef> controls, std::string_view name) {
    for (const ControlDef &control : controls) {
      if (SameWord_(control.name, name)) { return &control; }
      if (const ControlDef *below = Within_(control.children, name); below != nullptr) {
        return below;
      }
    }
    return nullptr;
  }

  static const ControlDef *WithField_(std::span<const ControlDef> controls, ::agiru::FieldNo no) {
    for (const ControlDef &control : controls) {
      if (control.kind == ControlKind::Field && control.field == no) { return &control; }
      if (const ControlDef *below = WithField_(control.children, no); below != nullptr) {
        return below;
      }
    }
    return nullptr;
  }

  [[nodiscard]] const ControlDef *ControlNamed_(std::string_view name) const {
    if constexpr (requires { PageTraits<P>::kPage; }) {
      if (const ControlDef *found = Within_(PageTraits<P>::kPage.layout, name); found != nullptr) {
        return found;
      }
      return Within_(PageTraits<P>::kPage.actions, name);
    } else {
      static_cast<void>(name);
      return nullptr;
    }
  }

  [[nodiscard]] const ControlDef *ControlByField_(::agiru::FieldNo no) const {
    if constexpr (requires { PageTraits<P>::kPage; }) {
      return WithField_(PageTraits<P>::kPage.layout, no);
    } else {
      static_cast<void>(no);
      return nullptr;
    }
  }

  P *page_ = nullptr;
  bool owned_ = false;
  PageCore *parent_ = nullptr;
  std::string partName_;
};

}
