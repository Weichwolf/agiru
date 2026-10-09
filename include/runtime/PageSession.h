#pragma once

#include "meta/PageDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/PageCore.h"
#include "runtime/PageInstance.h"
#include "runtime/PageValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/RecordWindow.h"
#include "runtime/Relation.h"
#include "runtime/SubPageLink.h"
#include "runtime/Table.h"
#include "runtime/TemporaryRecord.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/StringValue.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

/// \file
/// \brief Typed page lifecycle and control execution shared by production and AL test adapters.

namespace agiru {

template <typename P> class PageWindowSession;

/// \brief Production diagnostic policy; test error collection is an explicit adapter policy.
struct PageSessionDiagnostics {
  static constexpr std::string_view kNotOpen = "Page session is not open."; ///< Closed session.
  static constexpr std::string_view kNotOpenCode =
      "PageNotOpen"; ///< Closed-session classification.
  static constexpr std::string_view kAlreadyOpen =
      "Page session is already open."; ///< Double open.
  static constexpr std::string_view kAlreadyOpenCode =
      "PageAlreadyOpen"; ///< Double-open classification.
  static constexpr std::string_view kValidationCode =
      "PageValidation";                             ///< Unclassified validation.
  static constexpr bool kCollectSaveErrors = false; ///< Production row-save errors propagate.
};

/// \brief A typed, presentation-independent page with session-private execution state.
/// \tparam P The generated AL page or request-page class.
/// \tparam Diagnostics Explicit production/test error policy.
/// Client commands must use PageDispatcher and the owning authenticated session's
/// transaction boundary. This primitive does not authenticate, suspend modals or own
/// a database connection. Only specialized test adapters may copy/share its handle.
template <typename P, typename Diagnostics = PageSessionDiagnostics>
class PageSession : public PageCore {
public:
  /// \brief Whether the page has a source table; a dialog with none has no rows to move over.
  static constexpr bool kHasRecord =
      requires(P &page) { page.Rec.ValidateText(::agiru::FieldNo{}, std::string_view{}); };

  /// \brief Starts closed; construction does not execute an AL trigger.
  PageSession() = default;

protected:
  /// \brief A copy is a second HANDLE on the same page, for the AL TestPage adapter.
  /// \param o The harness copied.
  PageSession(const PageSession &o) { Share_(o); }

  /// \brief The same, by assignment.
  /// \param o The harness copied.
  /// \return This.
  PageSession &operator=(const PageSession &o) {
    if (this != &o) {
      Release_();
      Share_(o);
      if (page_ != nullptr) { Bind_(); }
    }
    return *this;
  }

  /// \brief A move takes the page over, ownership and all; `Clear(TestPage)` is one.
  /// \param o The harness moved from, which holds nothing afterwards.
  PageSession(PageSession &&o) noexcept { Take_(o); }

  /// \brief The same, by assignment.
  /// \param o The harness moved from.
  /// \return This.
  PageSession &operator=(PageSession &&o) noexcept {
    if (this != &o) {
      Release_();
      Take_(o);
      if (page_ != nullptr) { Bind_(); }
    }
    return *this;
  }

public:
  /// \brief Releases owned storage; explicit Close is required for AL save/close triggers.
  ~PageSession() override { Release_(); }

  /// \brief Whether this adapter currently holds an open page instance.
  [[nodiscard]] bool IsOpen() const { return page_ != nullptr; }

  /// \brief Positions a production adapter using the same record/trigger kernel as GoToRecord.
  /// \param record The exact table/primary-key identity.
  /// \return Whether it was found; the interior underscore cannot shadow an AL control name.
  [[nodiscard]] bool Select_Record(const RecordId &record) {
    if constexpr (kHasRecord) {
      return Landed_([&record](auto &rec) { return Platform_(rec).Get(record); });
    } else {
      static_cast<void>(record);
      throw Error("The page has no source record.", "PageRecord");
    }
  }

  /// \brief Reads the current source record identity for a production adapter.
  /// \return The exact identity, or empty when this page has no source table.
  /// \note A new record's key is not proof of persistence; this is not an AL control method.
  [[nodiscard]] RecordId Current_Record() const {
    static_cast<void>(Page_());
    if constexpr (kHasRecord) {
      return Record_().RecordId();
    } else {
      return {};
    }
  }

  /// \brief `PageSession.OpenNew()` -- opens the page on a new record.
  /// \throws Error when the page is already open, as AL does.
  void OpenNew() { Open_(true, true); }

  /// \brief `PageSession.OpenEdit()` -- opens the page on its first record, for editing.
  /// \throws Error when the page is already open, as AL does.
  void OpenEdit() { Open_(true, false); }

  /// \brief `PageSession.OpenView()` -- opens the page on its first record, read-only.
  /// \throws Error when the page is already open, as AL does.
  void OpenView() { Open_(false, false); }

  /// \brief Prepares a closed compiler-owned AL page without cloning its record or variables.
  /// \param page The original AL variable; remains alive until this adapter is released.
  /// \throws Error for double preparation or an already open adapter.
  /// \note Only the production factory uses this; it is not an AL TestPage operation.
  void Prepare_Borrowed(P &page) {
    if (page_ != nullptr || prepared_ != nullptr) {
      throw Error(Diagnostics::kAlreadyOpen, Diagnostics::kAlreadyOpenCode);
    }
    prepared_ = &page;
  }

  /// \brief Saves/closes with the explicit AL action while retaining refused close attempts.
  /// \param action The actual client choice. \return Existing LookupMode-normalized action.
  /// \throws Error for invalid choices or AL close/save failures; no implicit transaction change.
  [[nodiscard]] Action Close_Modal(Action action) {
    if (action < Action::OK || action > Action::RunSystem) {
      throw Error("Unknown modal close action.", "UiModalAction");
    }
    auto &page = Page_();
    static_cast<Page<P> &>(page).CloseWith(action);
    const Action result = static_cast<Page<P> &>(page).ClosedWith();
    Close_(action != Action::Cancel && action != Action::LookupCancel && action != Action::No);
    return result;
  }

  /// \brief `PageSession.Close()` -- runs `OnQueryClosePage` and `OnClosePage`, then lets go.
  void Close() { Close_(true); }

  /// \brief `PageSession.First()`.
  /// \return Whether there is a first record.
  Boolean First() {
    return Landed_([this](auto &) { return detail::FindPageRecord(Page_(), "-"); });
  }

  /// \brief `PageSession.Next()`.
  /// \return Whether there is a next record.
  Boolean Next() {
    const Boolean moved =
        Landed_([this](auto &) { return detail::NextPageRecord(Page_(), 1) != 0; });
    if (!moved) { PresentNewRow_(); }
    return moved;
  }

  /// \brief `PageSession.Previous()`.
  /// \return Whether there is a previous record.
  Boolean Previous() {
    return Landed_([this](auto &) { return detail::NextPageRecord(Page_(), -1) != 0; });
  }

  /// \brief `PageSession.Prev()`, the older spelling of `Previous`.
  /// \return Whether there is a previous record.
  Boolean Prev() { return Previous(); }

  /// \brief `PageSession.Last()`.
  /// \return Whether there is a last record.
  Boolean Last() {
    return Landed_([this](auto &) { return detail::FindPageRecord(Page_(), "+"); });
  }

  /// \brief `PageSession.New()` -- moves to a new record and runs `OnNewRecord`.
  void New() {
    if constexpr (kHasRecord) {
      static_cast<void>(Page_());
      SaveEditedNewRecord_();
      Relink_();
      const std::optional<std::int64_t> follows = StandingKey_();
      Platform_(Record_()).Init();
      detail::SeedFromFilters(
          static_cast<void *>(&Record_()), RecordTraits_().kTable, PopulateAllFields_());
      SplitKey_(follows);
      newRecord_ = true;
      detail::StartNewRecord(Page_(), false);
      detail::AfterGetRecord(Page_());
    } else {
      Unopened_();
    }
  }

  /// STEPPING PAST THE LAST ROW OF AN EDITABLE LIST LANDS ON THE BLANK ROW the client carries
  /// there, which is how `Lines.Last(); Lines.Next(); Lines."No.".SetValue(...)` adds a line
  /// (the aggregate codeunits' `CreateLineThroughTestPage`, 2026-09-12). Without it the
  /// `SetValue` edited the last existing line. The row is numbered like `New()`'s and is written
  /// when it is left, edited; a page that cannot insert stays where it is.
  void PresentNewRow_() {
    if constexpr (kHasRecord) {
      if (page_ == nullptr || newRecord_ || !Editable_()) { return; }
      if constexpr (requires { page_->OpenedEditable(); }) {
        if (!page_->OpenedEditable()) { return; }
      }
      const std::optional<std::int64_t> follows = StandingKey_();
      Platform_(Record_()).Init();
      detail::SeedFromFilters(
          static_cast<void *>(&Record_()), RecordTraits_().kTable, PopulateAllFields_());
      SplitKey_(follows);
      newRecord_ = true;
      detail::StartNewRecord(Page_(), true);
      detail::AfterGetRecord(Page_());
    }
  }

  /// The last primary-key field of the page's table when `AutoSplitKey` can number it -- an
  /// Integer or BigInteger (`devenv-autosplitkey-property.md` allows Guid and Decimal too, which
  /// wait for a case) -- or nothing.
  [[nodiscard]] static const FieldDef *SplitField_() {
    if constexpr (kHasRecord && requires { PageTraits<P>::kPage.autoSplitKey; }) {
      if (!PageTraits<P>::kPage.autoSplitKey) { return nullptr; }
      const TableDef &table = RecordTraits_().kTable;
      if (table.keys.empty() || table.keys[0].fields.empty()) { return nullptr; }
      const FieldDef *last = Field(table, table.keys[0].fields.back());
      if (last == nullptr ||
          (last->type != FieldType::Integer && last->type != FieldType::BigInteger)) {
        return nullptr;
      }
      return last;
    } else {
      return nullptr;
    }
  }

  /// The split key of the row the page stands on, when it stands on a saved one.
  [[nodiscard]] std::optional<std::int64_t> StandingKey_() {
    if constexpr (kHasRecord) {
      const FieldDef *last = SplitField_();
      if (last == nullptr || newRecord_) { return std::nullopt; }
      const detail::RecordState *state =
          reinterpret_cast<const detail::StateHandle *>(&Record_())->Peek();
      if (state == nullptr || !state->positioned) { return std::nullopt; }
      return KeyOf_(Record_(), *last);
    } else {
      return std::nullopt;
    }
  }

  template <typename R>
  [[nodiscard]] static std::optional<std::int64_t> KeyOf_(const R &rec, const FieldDef &last) {
    const std::string text = ::agiru::FieldText(static_cast<const void *>(&rec), last);
    if (text.empty()) { return 0; }
    try {
      return std::stoll(text);
    } catch (const std::exception &) { return std::nullopt; }
  }

  /// `AutoSplitKey` (`devenv-autosplitkey-property.md`, board:0354): "a value is automatically
  /// calculated for the last field of the primary key when a new record is inserted between two
  /// existing records. The new key value is set to a value halfway between the keys of the
  /// surrounding records"; past the last row it is that row's key plus ten thousand, and on an
  /// empty list ten thousand -- BC's document lines run 10000, 20000, 30000 for exactly this
  /// reason. Two lines added through a PageSession both took the key the filters seeded, 0, and the
  /// second could not be written (the aggregate codeunits, 2026-09-12).
  void SplitKey_(const std::optional<std::int64_t> &follows) {
    if constexpr (kHasRecord) {
      const FieldDef *last = SplitField_();
      if (last == nullptr) { return; }
      constexpr std::int64_t kStep = 10000;
      auto &rec = Record_();
      using Source = std::remove_cvref_t<decltype(rec)>;
      Source probe;
      static_cast<typename Source::Platform_Half &>(probe).Copy(rec);
      auto &walk = static_cast<typename Source::Platform_Half &>(probe);
      std::optional<std::int64_t> before = follows;
      std::optional<std::int64_t> after;
      if (before.has_value()) {
        ::agiru::detail::SetFieldText(static_cast<void *>(&probe), *last, std::to_string(*before));
        if (static_cast<bool>(walk.Find(">"))) { after = KeyOf_(probe, *last); }
      } else if (static_cast<bool>(walk.FindLast())) {
        before = KeyOf_(probe, *last);
      }
      std::int64_t value = kStep;
      if (before.has_value() && after.has_value()) {
        value = *after - *before >= 2 ? *before + (*after - *before) / 2 : *before + 1;
      } else if (before.has_value()) {
        value = *before + kStep;
      }
      ::agiru::detail::SetFieldText(static_cast<void *>(&rec), *last, std::to_string(value));
    } else {
      static_cast<void>(follows);
    }
  }

  /// \brief `PageSession.GoToRecord(Record)` -- positions the page on that record.
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

  /// \brief `PageSession.GoToKey(Values, ...)` -- positions the page on the record with that
  /// key.
  /// \tparam Values The key values' types.
  /// \param values The key values, in key order.
  /// \return Whether it was found.
  template <typename... Values> Boolean GoToKey(const Values &...values) {
    return Landed_([&](auto &rec) { return Platform_(rec).Get(values...); });
  }

  /// \brief Takes a page another runner opened, without owning it; a `[PageHandler]` gets one.
  /// \param page The page object, opened already.
  void Adopt(void *page) { AdoptObject(page, false); }

  /// \brief `PageSession.Caption()` -- what the title bar shows: the page's caption, and after
  ///        ` - ` its data caption where it has one.
  /// \return The caption.
  ///
  /// \note THE DATA CAPTION IS `DataCaptionExpression` WHERE THE PAGE DECLARES ONE and otherwise
  ///       what `DataCaptionFields` composes (`detail::PageDataCaption`). The shape is BC's own,
  ///       read off the tests that compare it whole: `ERM Sales/Purchase Application` expects
  ///       `'%1 - %2-%3'` of `Sales Journals`, the batch name and its description, which is the
  ///       page caption, ` - ` and `Rec.DataCaption()`. `CurrPage.Caption(...)` set at run time
  ///       replaces the caption half.
  [[nodiscard]] Text<0> Caption() const {
    if constexpr (requires { PageTraits<P>::kPage; }) {
      std::string caption =
          page_ != nullptr ? static_cast<const Page<P> &>(*page_).Caption() : std::string{};
      if (caption.empty()) {
        caption = std::string(PageTraits<P>::kPage.caption.empty() ? PageTraits<P>::kName
                                                                   : PageTraits<P>::kPage.caption);
      }
      std::string data;
      if (page_ != nullptr) {
        if constexpr (requires { page_->OnDataCaptionExpression(); }) {
          data = std::string(std::string_view(page_->OnDataCaptionExpression()));
        } else if constexpr (kHasRecord) {
          data = detail::PageDataCaption(static_cast<const void *>(&Record_()),
                                         RecordTraits_().kTable,
                                         PageTraits<P>::kPage.type,
                                         PageTraits<P>::kPage.dataCaptionFields);
        }
      }
      return Text<0>{data.empty() ? caption : caption + " - " + data};
    } else {
      Unopened_();
    }
  }

  /// \brief `PageSession.Editable()`.
  /// \return Whether the page was opened for editing.
  [[nodiscard]] Boolean Editable() const {
    if constexpr (requires(P &page) { page.OpenedEditable(); }) {
      return page_ != nullptr && page_->OpenedEditable();
    } else {
      return false;
    }
  }

  /// \brief `PageSession.GetValidationError([Index])` -- what a row's write refused with
  ///        (`testpage-getvalidationerror-method.md`: "the list of all validation errors that
  ///        occurred on a test page").
  /// \param Index One-based; the last error when omitted.
  /// \return The error's text, empty when there is none.
  ///
  /// \note AN ERROR RAISED WHILE THE USER LEAVES A ROW IS THE PAGE'S, NOT THE TEST'S: the client
  ///       shows it beside the row and the test goes on to its own assertion. `VAT Return Period
  ///       List.OnInsertRecord` refuses a manual row with `Error('')`, and the test asserts
  ///       afterwards that no row exists (`UI_ManualInsert_TypedManualReceiveCU`, 2026-09-12); the
  ///       predecessor checked the W1 suite for the opposite expectation and found none -- every
  ///       `asserterror Close()`/`New()` there expects the test FRAMEWORK's own message. A
  ///       `SetValue`'s validation error still throws, coded `TestValidation`, as before.
  [[nodiscard]] Text<0> GetValidationError(Integer Index = 0) const {
    if (validationErrors_.empty()) { return Text<0>{}; }
    const std::size_t at =
        Index <= 0 ? validationErrors_.size() - 1 : static_cast<std::size_t>(Index) - 1;
    return at < validationErrors_.size() ? Text<0>{validationErrors_[at]} : Text<0>{};
  }

  /// \brief `PageSession.ValidationErrorCount()`. \return How many rows' writes were refused.
  [[nodiscard]] Integer ValidationErrorCount() const {
    return static_cast<Integer>(validationErrors_.size());
  }

  /// A READ-ONLY CONTROL THAT SHOWS AN EXPRESSION TAKES A `SetValue` AND DOES NOTHING WITH IT:
  /// `field("Price Line Filter"; GetReadablePriceLineFilter()) { Editable = false; }` has no
  /// field and no variable to write, and BC's own suite sets it inside a `[ModalPageHandler]`
  /// and goes on (Suggest Price Lines UT `T005`, 2026-09-12) -- the client would have shown no
  /// editor at all. An EDITABLE expression control still refuses, because there the value has to
  /// go somewhere and the page has not said where.
  void SetControlText(std::string_view control, std::string_view text) override {
    static_cast<void>(Page_());
    Relink_();
    const ControlDef *def = ControlNamed_(control);
    if (def != nullptr && def->field.Value() == 0) {
      if (const ControlTrigger<P> *row = TriggerRow_(control);
          row != nullptr && row->set != nullptr) {
        SaveEditedNewRecord_();
        try {
          detail::CheckEntryRange(text, def->minValue, def->maxValue);
        } catch (const Error &e) { throw e.Coded(Diagnostics::kValidationCode); }
        row->set(Page_(), text);
        RunSavingIfValidated_(
            [this, control] { RunTrigger_(control, ControlTriggerKind::Validate, true); });
        return;
      }
    }
    if (def != nullptr && def->field.Value() == 0 && !static_cast<bool>(ControlEditable(control))) {
      return;
    }
    if (def == nullptr || def->field.Value() == 0) {
      throw Error("the control '" + std::string(control) + "' shows no field to set");
    }
    if constexpr (kHasRecord) {
      PrepareRecordEdit_();
      const detail::ValidatingField editing(def->field);
      auto before = Record_();
      if constexpr (requires { page_->MarkEdited(); }) { page_->MarkEdited(); }
      try {
        Record_().CheckEntryRange(def->field, text);
        PageValidateEvent_("OnBeforeValidateEvent", def->name, before);
        Record_().ValidateText(def->field, text);
        if (text.empty() && Record_().FieldNotBlank(def->field)) {
          try {
            Record_().TestField(def->field);
          } catch (const Error &e) { throw Error(e.what(), Diagnostics::kValidationCode); }
        }
        RunTrigger_(control, ControlTriggerKind::Validate, true);
        PageValidateEvent_("OnAfterValidateEvent", def->name, before);
      } catch (const Error &e) {
        Record_() = before;
        throw e.Coded(Diagnostics::kValidationCode);
      }
      edited_ = true;
      SaveExistingRecord_();
    } else {
      static_cast<void>(text);
      Unopened_();
    }
  }

  /// A CONTROL'S OWN TRIGGER SAVES THE ROW TOO, WHEN IT VALIDATED THE RECORD. `field("Recurring
  /// Frequency"; RecurringFrequency)` writes nothing itself; its `OnValidate` runs `Rec.Validate(
  /// "Recurring Frequency", ...)`, an `OnLookup` that answers false may still have run
  /// `Validate("Item No.", ...)` on the row (`Item Reference Management`), and
  /// `devenv-onmodifyrecord-page-trigger.md` has the platform write the changed row the same way
  /// it writes one a field control changed. Not every such trigger edits: most variable controls
  /// show a sum or a filter, and a journal's batch control MOVES the record to another row. So
  /// the row is saved only when a `Validate` ran inside the trigger, the key is the one the page
  /// stood on, and a field differs -- the three guards the predecessor measured on
  /// `BatchNameLookup_SaveRecord_OnlyWhenNotEmptyLine` (openerp
  /// `_zeile_schmutzig_wenn_satz_veraendert`; ERM General Journal UT's recurring-frequency cases
  /// and Phys. Invt. Order Line TAB UT's reference lookups, 2026-09-12).
  template <typename Run> void RunSavingIfValidated_(Run &&run) {
    if constexpr (kHasRecord) {
      if (page_ == nullptr) {
        run();
        return;
      }
      const auto before = Record_();
      const std::size_t taken = detail::BeforeImagesTaken();
      run();
      SaveIfValidated_(before, taken);
    } else {
      run();
    }
  }

  template <typename R> void SaveIfValidated_(const R &before, std::size_t taken) {
    if constexpr (kHasRecord) {
      if (page_ == nullptr || detail::BeforeImagesTaken() == taken) { return; }
      if (detail::SameFields(before, Record_())) { return; }
      if (!newRecord_ && !detail::SameKey(before, Record_())) { return; }
      edited_ = true;
      SaveExistingRecord_();
    } else {
      static_cast<void>(before);
      static_cast<void>(taken);
    }
  }

  /// THE WRITE HALF OF A `SetValue` ROUND TRIP. The client saves a record's change as soon as the
  /// user leaves the field (`teams-faq.md`: "automatically saves changes you make to any field as
  /// soon as you leave the field"), and a `TestField.SetValue` is that entry and that leave: the
  /// page's `OnModifyRecord` first, which may decline the platform's write by answering false,
  /// then `Modify(true)`. A NEW record waits for `SaveNewRecord_`, and a page standing on no
  /// record has nothing to save. Openerp WI-1113 measured the round trip at +17.
  void SaveExistingRecord_() {
    if constexpr (kHasRecord) {
      ReconcileNewRecord_();
      if (newRecord_ || page_ == nullptr) { return; }
      const detail::RecordState *state =
          reinterpret_cast<const detail::StateHandle *>(&Record_())->Peek();
      if (state == nullptr || !state->positioned) { return; }
      if constexpr (requires { page_->ModifyRecord(); }) {
        static_cast<void>(page_->ModifyRecord());
      }
    }
  }

  /// \brief Prepares a positioned SQL record for input without refreshing its observed version.
  /// \note A committed peer change must conflict at the atomic write predicate, not silently
  /// authorize the stale client input through a reread. Shown FlowFields are calculated without
  /// repeating page triggers. New and temporary records retain their existing lifecycle.
  void PrepareRecordEdit_() {
    if constexpr (kHasRecord) {
      ReconcileNewRecord_();
      if (newRecord_ || page_ == nullptr) { return; }
      const detail::RecordState *state =
          reinterpret_cast<const detail::StateHandle *>(&Record_())->Peek();
      if (state == nullptr || !state->positioned) { return; }
      if (detail::RuntimeIsTemporary(&Record_())) { return; }
      if constexpr (requires { PageTraits<P>::kPage.layout; }) {
        detail::CalcShownFlowFields(
            static_cast<void *>(&Record_()), RecordTraits_().kTable, PageTraits<P>::kPage.layout);
      }
    }
  }

  /// THE READ HALF AFTER AN ACTION: the page re-reads the record it stands on, because the action
  /// may have written it through a variable of its own (`VAT Return Period Card`'s "Create VAT
  /// Return" writes the period's return number that way and the card showed the old one, 6 UT
  /// cases), and the after-get triggers run again the way the refreshed page runs them. A record
  /// the action deleted stays as it was, the way the client keeps showing it.
  void RereadAfterAction_() {
    if constexpr (kHasRecord) {
      if (newRecord_ || page_ == nullptr) { return; }
      const detail::RecordState *state =
          reinterpret_cast<const detail::StateHandle *>(&Record_())->Peek();
      if (state == nullptr || !state->positioned) { return; }
      if (static_cast<bool>(Platform_(Record_()).Find("="))) { detail::AfterGetRecord(*page_); }
    }
  }

  /// A part read through a `const` path attaches on first use like any other use of it, and
  /// follows the parent before it answers.
  void AttachedForReading_() const {
    if (parent_ == nullptr) { return; }
    auto &self = *const_cast<PageSession *>(this);
    if (page_ == nullptr) { self.Attach_(); }
    self.Relink_();
  }

  [[nodiscard]] std::string ControlText(std::string_view control) const override {
    AttachedForReading_();
    const ControlDef *def = ControlNamed_(control);
    if (def != nullptr && def->field.Value() == 0 && page_ != nullptr) {
      if (const ControlTrigger<P> *row = TriggerRow_(control); row != nullptr) {
        if (row->sourceText != nullptr) {
          return std::string(((*page_).*row->sourceText)().Value());
        }
        if (row->text != nullptr) { return row->text(*page_); }
      }
    }
    if (def == nullptr || def->field.Value() == 0) {
      throw Error("the control '" + std::string(control) + "' shows no field to read");
    }
    if constexpr (kHasRecord) {
      return Record_().FieldFormat(def->field);
    } else {
      Unopened_();
    }
  }

  [[nodiscard]] std::string ControlOrdinal(std::string_view control) const override {
    AttachedForReading_();
    const ControlDef *def = ControlNamed_(control);
    if (def == nullptr || def->field.Value() == 0) { return {}; }
    if constexpr (kHasRecord) {
      return detail::FieldOrdinalText(&Record_(), RecordTraits_().kTable, def->field);
    } else {
      return {};
    }
  }

  [[nodiscard]] PageValue Control_Value(std::string_view control) const override {
    AttachedForReading_();
    static_cast<void>(Page_());
    const ControlDef *def = ControlNamed_(control);
    if (const ControlTrigger<P> *row = TriggerRow_(control);
        row != nullptr && row->value != nullptr) {
      return row->value(*page_);
    }
    if constexpr (kHasRecord) {
      if (def != nullptr && def->field.Value() != 0) {
        const auto &table = RecordTraits_().kTable;
        if (const FieldDef *field = Field(table, def->field)) {
          return ReadPageValue(&Record_(), table, *field);
        }
      }
    }
    return PageCore::Control_Value(control);
  }

  void RunControlTrigger(std::string_view control, ControlTriggerKind kind) override {
    if (kind == ControlTriggerKind::Action && CloseAction_(control)) { return; }
    if (kind == ControlTriggerKind::Action && ModeAction_(control)) { return; }
    if (kind == ControlTriggerKind::Action && page_ != nullptr) {
      SaveEditedNewRecord_();
      detail::RaisePageRecordEvent(*page_, "OnBeforeActionEvent", control);
    }
    RunTrigger_(control, kind, kind == ControlTriggerKind::Action);
    if (kind == ControlTriggerKind::Action && page_ != nullptr) {
      detail::RaisePageRecordEvent(*page_, "OnAfterActionEvent", control);
      if (ClosedItself_()) { return; }
      RereadAfterAction_();
    }
  }

  /// `CurrPage.Close()` INSIDE A TRIGGER closes the page once the trigger returns: the close
  /// triggers run and the harness lets go, so the next `OpenEdit` on this variable opens a fresh
  /// page (`Purchase Journal.ClassicView` closes the simple view and runs the classic one; ERM
  /// General Journal UT reopens the simple one afterwards, 2 cases, 2026-09-12).
  bool ClosedItself_() {
    if constexpr (requires { page_->Closed(); }) {
      if (page_ == nullptr || !page_->Closed()) { return false; }
      for (PageCore *part : parts_) { part->RowLeft(); }
      edited_ = false;
      detail::ClosePage(*page_);
      Release_();
      return true;
    } else {
      return false;
    }
  }

  [[nodiscard]] ::agiru::FieldNo ControlFieldNumber_(std::string_view control) const {
    const ControlDef *def = ControlNamed_(control);
    ::agiru::FieldNo no = def != nullptr ? def->field : ::agiru::FieldNo{};
    if constexpr (kHasRecord) {
      if (no.Value() == 0) {
        for (const FieldDef &field : RecordTraits_().kTable.fields) {
          if (SameWord_(field.name, control)) { no = field.no; }
        }
      }
    }
    return no;
  }

  [[nodiscard]] std::string ControlFilterText(std::string_view control) const override {
    if constexpr (kHasRecord) {
      if (page_ == nullptr) { return {}; }
      const ::agiru::FieldNo no = ControlFieldNumber_(control);
      if (no.Value() == 0) { return {}; }
      const detail::RecordState *state =
          reinterpret_cast<const detail::StateHandle *>(&Record_())->Peek();
      if (state == nullptr) { return {}; }
      for (const detail::FieldFilter &one : state->filters) {
        if (one.field == no && one.group == state->group) {
          const FieldDef *shown = ::agiru::Field(RecordTraits_().kTable, no);
          return shown == nullptr ? std::string(one.text) : detail::ShownFilter(*shown, one.text);
        }
      }
      return {};
    } else {
      static_cast<void>(control);
      return {};
    }
  }

  void SetControlFilter(std::string_view control, std::string_view filter) override {
    if constexpr (kHasRecord) {
      const ::agiru::FieldNo no = ControlFieldNumber_(control);
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
    AttachedForReading_();
    if constexpr (requires { PageTraits<P>::kPage; }) {
      const int within = VisibleWithin_(PageTraits<P>::kPage.layout, control);
      if (within >= 0) { return within == 1; }
      const int action = VisibleWithin_(PageTraits<P>::kPage.actions, control);
      if (action >= 0) { return action == 1; }
    }
    return OwnVisible_(control, ControlNamed_(control));
  }

  /// A control is visible when it is and every container above it is; the client shows nothing
  /// of a hidden group. Returns 1 for visible, 0 for hidden, -1 when the name is not here.
  [[nodiscard]] int VisibleWithin_(std::span<const ControlDef> controls,
                                   std::string_view name) const {
    for (const ControlDef &control : controls) {
      if (SameWord_(control.name, name)) { return OwnVisible_(control.name, &control) ? 1 : 0; }
      const int below = VisibleWithin_(control.children, name);
      if (below < 0) { continue; }
      if (below == 0) { return 0; }
      return OwnVisible_(control.name, &control) ? 1 : 0;
    }
    return -1;
  }

  [[nodiscard]] bool OwnVisible_(std::string_view name, const ControlDef *def) const {
    if (const auto computed = Computed_(name, &ControlTrigger<P>::visible); computed) {
      return static_cast<bool>(*computed);
    }
    return def == nullptr || !SameWord_(def->visible, "false");
  }

  /// A CONTROL IS EDITABLE WHEN IT IS AND EVERY CONTAINER ABOVE IT IS, the way `Visible` already
  /// walks the tree: `VAT Return Period List` puts `Editable = IsEditable` on its repeater and
  /// nothing on the fields, and every field answered editable (3 UT cases, 2026-09-12). The
  /// same holds for `Enabled`.
  [[nodiscard]] Boolean ControlEditable(std::string_view control) const override {
    AttachedForReading_();
    if (!Editable()) { return false; }
    if constexpr (requires { PageTraits<P>::kPage; }) {
      const int within = StateWithin_(PageTraits<P>::kPage.layout, control, true);
      if (within >= 0) { return within == 1; }
    }
    return OwnState_(control, ControlNamed_(control), true);
  }

  [[nodiscard]] Boolean ControlEnabled(std::string_view control) const override {
    AttachedForReading_();
    if constexpr (requires { PageTraits<P>::kPage; }) {
      const int within = StateWithin_(PageTraits<P>::kPage.layout, control, false);
      if (within >= 0) { return within == 1; }
      const int action = StateWithin_(PageTraits<P>::kPage.actions, control, false);
      if (action >= 0) { return action == 1; }
    }
    return OwnState_(control, ControlNamed_(control), false);
  }

  /// Returns 1 for editable (enabled), 0 for not, -1 when the name is not in this tree.
  [[nodiscard]] int
  StateWithin_(std::span<const ControlDef> controls, std::string_view name, bool editable) const {
    for (const ControlDef &control : controls) {
      if (SameWord_(control.name, name)) {
        return OwnState_(control.name, &control, editable) ? 1 : 0;
      }
      const int below = StateWithin_(control.children, name, editable);
      if (below < 0) { continue; }
      if (below == 0) { return 0; }
      return OwnState_(control.name, &control, editable) ? 1 : 0;
    }
    return -1;
  }

  [[nodiscard]] bool OwnState_(std::string_view name, const ControlDef *def, bool editable) const {
    if (const auto computed =
            Computed_(name, editable ? &ControlTrigger<P>::editable : &ControlTrigger<P>::enabled);
        computed) {
      return static_cast<bool>(*computed);
    }
    return def == nullptr || !SameWord_(editable ? def->editable : def->enabled, "false");
  }

  [[nodiscard]] std::optional<Boolean>
  Computed_(std::string_view control, ::agiru::Boolean (P::*ControlTrigger<P>::*state)()) const {
    if constexpr (requires { PageTraits<P>::kControlTriggers; }) {
      if (page_ == nullptr) { return std::nullopt; }
      for (const ControlTrigger<P> &trigger : PageTraits<P>::kControlTriggers) {
        if (!SameWord_(trigger.control, control)) { continue; }
        const auto compute = trigger.*state;
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
    parent.AttachPart(*this);
    Bind_();
  }

  [[nodiscard]] void *PartInstance(std::string_view control) override {
    if constexpr (requires(P &page) { page.PartInstance(control); }) {
      return page_ == nullptr ? nullptr : page_->PartInstance(control);
    } else {
      return nullptr;
    }
  }

  void RowLeft() override { SaveEditedNewRecord_(); }

  void AttachPart(PageCore &part) override {
    for (PageCore *held : parts_) {
      if (held == &part) { return; }
    }
    parts_.push_back(&part);
  }

  void LinkPart(std::string_view control, void *subRecord, const TableDef &subTable) override {
    if constexpr (kHasRecord) {
      SaveNewRecord_();
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
    throw Error(Diagnostics::kNotOpen, Diagnostics::kNotOpenCode);
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

  /// A PART FOLLOWS ITS PARENT, and following means the part's current record moves with the
  /// link and `OnAfterGetRecord` runs for it -- unless the part stands on a NEW record, which
  /// `New()` made and nothing has inserted yet: a re-find would move it onto an existing row
  /// (`Price List Line UT.VariantCodeMustBeBlankWhenInsertNewRecord`, chain 88, 2026-09-10).
  /// link and `OnAfterGetRecord` runs for it: `Whse. Pick Subform` sets `BinCodeEditable` there,
  /// and `WarehousePick.WhseActivityLines."Bin Code".Editable()` read a stale one after the
  /// parent's filter moved (3 cases of SCM - Warehouse UT, 2026-09-10). A relink that only set
  /// the filters left the part on the row it opened on.
  /// \brief Writes a record `New()` or `OpenNew()` made, which is what BC does when focus leaves
  ///        the header: the part that follows reads a document with a number.
  /// \note IT IS THE PLATFORM'S SAVE AND NOT AN INSERT THE TEST ASKED FOR. `OnInsert` runs, so
  ///       the No. Series assigns the number the lines key on; a page that inserts nothing (a
  ///       list on a temporary table) simply has nothing to save.
  /// A NEW ROW THE USER EDITED IS INSERTED WHEN THE ROW IS LEFT
  /// (`devenv-delayedinsert-property.md`: the record is inserted "when the user leaves the row"),
  /// and a new row nobody edited is dropped the way the client drops it. Leaving is: setting a
  /// control that is not a field of the row (`Sales Invoice Subform."Invoice Discount Amount"`
  /// after the line's `Quantity`, 12 UT cases of the aggregate codeunits, 2026-09-12), another row,
  /// an action, `New`, `Close`.
  void SaveEditedNewRecord_() {
    for (PageCore *part : parts_) { part->RowLeft(); }
    if (!edited_) { return; }
    edited_ = false;
    try {
      SaveNewRecord_();
    } catch (const Error &e) {
      if constexpr (Diagnostics::kCollectSaveErrors) {
        validationErrors_.emplace_back(e.what());
      } else {
        throw;
      }
    }
  }

  /// \brief Reconciles source AL insertion with the client's pending-new-record marker.
  /// \note A nonblank SystemId alone is not proof: both its immutable identity and the
  ///       current primary key must exist in the current storage/transaction. The bounded
  ///       existence probe preserves the live buffer, filters, image and rowversion.
  ///       Pending, copied-to-another-key and rolled-back rows remain pending.
  void ReconcileNewRecord_() {
    if constexpr (kHasRecord) {
      if constexpr (requires { Record_().SystemId.IsNull(); }) {
        if (!newRecord_ || page_ == nullptr || Record_().SystemId.IsNull()) { return; }
        using Source = std::remove_cvref_t<decltype(Record_())>;
        Source probe = Record_();
        auto &platform = Platform_(probe);
        if (detail::RuntimeIsTemporary(&Record_())) {
          if (!detail::RuntimeGet(&probe, RecordTraits_().kTable) ||
              probe.SystemId != Record_().SystemId) {
            return;
          }
        } else {
          platform.Reset();
          platform.SetRecFilter();
          platform.SetRange(probe.SystemId, probe.SystemId);
          if (platform.IsEmpty()) { return; }
        }
        newRecord_ = false;
        page_->LandedOnRecord();
        reinterpret_cast<detail::StateHandle *>(&Record_())->Ensure().positioned = true;
      }
    }
  }

  void SaveNewRecord_() {
    if constexpr (kHasRecord) {
      ReconcileNewRecord_();
      if (!newRecord_ || page_ == nullptr) { return; }
      newRecord_ = false;
      edited_ = false;
      if constexpr (requires { page_->InsertNewRecord(); }) {
        static_cast<void>(page_->InsertNewRecord());
      }
    }
  }

  /// \brief Whether the page lets a row be written, which decides whether stepping past the last
  ///        one lands on the blank row BC's editable list carries.
  /// \return Whether it does.
  [[nodiscard]] bool Editable_() const {
    if (page_ == nullptr) { return false; }
    if constexpr (requires { PageTraits<P>::kPage; }) {
      const auto says = [](std::string_view property) {
        return property.empty() || !SameWord_(property, "false");
      };
      return says(PageTraits<P>::kPage.editable) && says(PageTraits<P>::kPage.insertAllowed);
    } else {
      return true;
    }
  }

  void Relink_() {
    if constexpr (kHasRecord) {
      if (parent_ != nullptr && page_ != nullptr) {
        parent_->LinkPart(partName_, static_cast<void *>(&page_->Rec), RecordTraits_().kTable);
        using Source = std::remove_cvref_t<decltype(page_->Rec)>;
        auto &platform = static_cast<typename Source::Platform_Half &>(page_->Rec);
        if constexpr (requires { PageTraits<P>::kPage; }) {
          detail::ApplyPageView(static_cast<void *>(&page_->Rec),
                                RecordTraits_().kTable,
                                PageTraits<P>::kPage.sourceTableView);
        }
        if (newRecord_) { return; }
        if (static_cast<bool>(platform.Find("=")) || static_cast<bool>(platform.FindFirst())) {
          page_->LandedOnRecord();
          detail::AfterGetRecord(*page_);
          return;
        }
        detail::SeedFromFilters(
            static_cast<void *>(&page_->Rec), RecordTraits_().kTable, PopulateAllFields_());
        SplitKey_(std::nullopt);
        newRecord_ = true;
        detail::StartNewRecord(*page_, false);
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

  [[nodiscard]] static bool PopulateAllFields_() {
    if constexpr (requires { PageTraits<P>::kPage; }) {
      return PageTraits<P>::kPage.populateAllFields;
    } else {
      return false;
    }
  }

  static auto RecordTraits_()
    requires kHasRecord
  {
    return TableTraits<std::remove_cvref_t<decltype(std::declval<P &>().Rec)>>{};
  }

  void Open_(bool editable, bool isNew) {
    OpenWith_(isNew, [editable, isNew](P &page) { detail::OpenPage(page, editable, isNew); });
  }

  template <typename Initialize> void OpenWith_(bool isNew, Initialize initialize) {
    if (page_ != nullptr) { throw Error(Diagnostics::kAlreadyOpen, Diagnostics::kAlreadyOpenCode); }
    if constexpr (requires { PageTraits<P>::kPage; }) {
      owned_ = prepared_ == nullptr;
      page_ = owned_ ? new P() : std::exchange(prepared_, nullptr);
      Bind_();
      try {
        initialize(*page_);
        newRecord_ = isNew;
        ReconcileNewRecord_();
      } catch (...) {
        Release_();
        throw;
      }
    } else {
      static_cast<void>(initialize);
      static_cast<void>(isNew);
      Unopened_();
    }
  }

  void Close_(bool save) {
    if (page_ == nullptr) { return; }
    if (save) {
      SaveEditedNewRecord_();
    } else {
      for (PageCore *part : parts_) { part->RowLeft(); }
      edited_ = false;
    }
    detail::ClosePage(*page_);
    Release_();
  }

  void Take_(PageSession &o) {
    prepared_ = std::exchange(o.prepared_, nullptr);
    page_ = o.page_;
    owned_ = o.owned_;
    o.page_ = nullptr;
    o.owned_ = false;
  }

  void Share_(const PageSession &o) {
    prepared_ = nullptr;
    page_ = o.page_;
    owned_ = false;
  }

  void Release_() {
    if (page_ != nullptr && owned_) { delete page_; }
    page_ = nullptr;
    owned_ = false;
    prepared_ = nullptr;
  }

  void Bind_() { BindPresentation(); }

  template <typename Step> Boolean Landed_(Step step) {
    if constexpr (kHasRecord) {
      static_cast<void>(Page_());
      SaveEditedNewRecord_();
      Relink_();
      newRecord_ = false;
      try {
        const bool found = step(Record_());
        if (found) {
          Page_().LandedOnRecord();
          detail::AfterGetRecord(Page_());
        }
        return found;
      } catch (...) {
        Release_();
        throw;
      }
    } else {
      static_cast<void>(step);
      Unopened_();
    }
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

  bool EditCard_() {
    if constexpr (kHasRecord && requires { PageTraits<P>::kPage; }) {
      const PageDef &list = PageTraits<P>::kPage;
      if ((list.type != PageType::List && list.type != PageType::ListPart) ||
          list.cardPageId.Value() == 0) {
        return false;
      }
      const PageEntry *entry = FindPage(list.cardPageId);
      if (entry == nullptr || entry->run == nullptr) {
        throw Error("system Edit: the declared CardPageId is not executable");
      }
      if (entry->page->source != RecordTraits_().kTable.id) {
        throw Error("system Edit: CardPageId has a different SourceTable");
      }
      if (detail::SaysFalse(entry->page->modifyAllowed)) {
        throw Error("system Edit: the card's ModifyAllowed property is false");
      }
      SaveEditedNewRecord_();
      const detail::RecordState *state =
          reinterpret_cast<const detail::StateHandle *>(&Record_())->Peek();
      if (state == nullptr || !state->positioned) { throw Error("system Edit: no current record"); }
      static_cast<void>(
          detail::RunPageByNumber(false, list.cardPageId.Value(), std::as_const(Record_())));
      return true;
    }
    return false;
  }

  bool ModeAction_(std::string_view control) {
    const bool edit = SameWord_(control, "Edit");
    const bool view = SameWord_(control, "View");
    if ((!edit && !view) || ControlNamed_(control) != nullptr || page_ == nullptr) { return false; }
    if (edit && EditCard_()) { return true; }
    if constexpr (requires(P &page) { page.OpenedAs(edit); }) { page_->OpenedAs(edit); }
    return true;
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
      if (owned_) {
        Close_(action != ::agiru::Action::Cancel && action != ::agiru::Action::No &&
               action != ::agiru::Action::LookupCancel);
      }
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

  [[nodiscard]] static const ControlTrigger<P> *TriggerRow_(std::string_view control) {
    if constexpr (requires { PageTraits<P>::kControlTriggers; }) {
      for (const ControlTrigger<P> &trigger : PageTraits<P>::kControlTriggers) {
        if (SameWord_(trigger.control, control)) { return &trigger; }
      }
    }
    return nullptr;
  }

  [[nodiscard]] static const FieldDef *RelationColumn_(const TableDef &table,
                                                       std::string_view name) {
    const FieldDef *column = nullptr;
    for (const FieldDef &candidate : table.fields) {
      if (name.empty() ? (!table.keys.empty() && !table.keys[0].fields.empty() &&
                          candidate.no == table.keys[0].fields[0])
                       : SameWord_(candidate.name, name)) {
        column = &candidate;
      }
    }
    return column;
  }

  static void FilterRelatedRecord_(void *record,
                                   const TableDef &table,
                                   std::span<const detail::RelationFilter> filters) {
    detail::RecordState &state = reinterpret_cast<detail::StateHandle *>(record)->Ensure();
    for (const detail::RelationFilter &filter : filters) {
      for (const FieldDef &candidate : table.fields) {
        if (SameWord_(candidate.name, filter.field)) {
          detail::Narrow(state, candidate.no, filter.text);
        }
      }
    }
  }

  /// AL `TestField.Lookup()` on a control with no `OnLookup` of its own opens the RELATED table's
  /// lookup page through the test's ModalPageHandler, and `LookupOK` puts the picked record's
  /// related field into the control (`devenv-tablerelation-property.md`, `FindLookupPage`).
  bool LookupThroughRelation_(std::string_view control) {
    if constexpr (kHasRecord) {
      const ControlDef *def = ControlNamed_(control);
      if (def == nullptr || def->field.Value() == 0) { return false; }
      if (Record_().RunOnLookup(def->field)) { return true; }
      const FieldDef *field = Field(RecordTraits_().kTable, def->field);
      if (field == nullptr) { return false; }
      const std::optional<detail::ResolvedRelation> resolved =
          detail::ResolveRelation(&Record_(), RecordTraits_().kTable, *field);
      if (!resolved.has_value()) { return false; }
      const TableEntry *target = FindTable(resolved->table);
      if (target == nullptr) { return false; }
      const PageEntry *lookup = FindLookupPage(*target->table);
      if (lookup == nullptr) { return false; }
      const FieldDef *column = RelationColumn_(*target->table, resolved->field);
      void *related = target->make();
      FilterRelatedRecord_(related, *target->table, resolved->filters);
      try {
        if (lookup->run(true, related, target->table, true) == ::agiru::Action::LookupOK &&
            column != nullptr) {
          SetControlText(control, ::agiru::FieldText(related, *column));
        }
      } catch (...) {
        target->free(related);
        throw;
      }
      target->free(related);
      return true;
    } else {
      static_cast<void>(control);
      return false;
    }
  }

  void RunLookupTrigger_(std::string_view control, const ControlTrigger<P> &trigger) {
    ::agiru::Text<0> text(ControlText(control));
    if constexpr (kHasRecord) {
      const auto before = Record_();
      const std::size_t taken = detail::BeforeImagesTaken();
      if (std::invoke(trigger.lookup, Page_(), text)) {
        SetControlText(control, text.Value());
      } else {
        SaveIfValidated_(before, taken);
      }
    } else {
      if (std::invoke(trigger.lookup, Page_(), text)) { SetControlText(control, text.Value()); }
    }
  }

  bool RunDeclaredTrigger_(std::string_view control,
                           ControlTriggerKind kind,
                           const ControlTrigger<P> &trigger) {
    if (kind == ControlTriggerKind::Lookup && trigger.lookup != nullptr) {
      RunLookupTrigger_(control, trigger);
      return true;
    }
    const auto run = Trigger_(trigger, kind);
    if (run == nullptr) { return false; }
    if (kind == ControlTriggerKind::DrillDown || kind == ControlTriggerKind::AssistEdit) {
      RunSavingIfValidated_([this, run] { (Page_().*run)(); });
    } else {
      (Page_().*run)();
    }
    return true;
  }

  void RunTrigger_(std::string_view control, ControlTriggerKind kind, bool optional) {
    const ControlTrigger<P> *trigger = TriggerRow_(control);
    if (trigger != nullptr && RunDeclaredTrigger_(control, kind, *trigger)) { return; }
    if (kind == ControlTriggerKind::Lookup && LookupThroughRelation_(control)) { return; }
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

protected:
  /// \brief Rebinds presentation-only control handles after opening, adoption or copying.
  virtual void BindPresentation() {}

  /// \brief Accesses the live typed page for a specialized adapter.
  /// \throws Error when the page is not open.
  P &Instance() { return Page_(); }

  /// \brief Finds a field declaration for an adapter's typed AL control handle.
  /// \param number The source table field number.
  /// \return The matching control, or nullptr.
  [[nodiscard]] const ControlDef *FindFieldControl(FieldNo number) const {
    return ControlByField_(number);
  }

  /// \brief Adopts an already-open page with explicit ownership.
  /// \param object The typed page object.
  /// \param owned Whether this session must destroy it.
  void AdoptObject(void *object, bool owned) {
    Release_();
    page_ = static_cast<P *>(object);
    owned_ = owned;
    Bind_();
  }

  /// \brief Raises the adapter's not-open diagnostic without touching a page.
  [[noreturn]] static void NotOpen() { Unopened_(); }

private:
  friend class PageWindowSession<P>;
  P *page_ = nullptr;
  P *prepared_ = nullptr;
  bool owned_ = false;
  bool newRecord_ = false;
  bool edited_ = false;
  std::vector<PageCore *> parts_;
  std::vector<std::string> validationErrors_;
  PageCore *parent_ = nullptr;
  std::string partName_;
};

/// \brief Production-only SQL list loading; adds no methods to the AL TestPage facade.
/// \tparam P The generated page class; custom/temporary providers remain explicit gaps.
template <typename P> class PageWindowSession final : public PageSession<P> {
  using Base = PageSession<P>;

public:
  /// \brief Initializes a SQL list and loads its first block without single-row triggers.
  /// \param mode View/Edit; New is not qualified by this adapter.
  /// \param limit Positive trusted server bound.
  /// \param receiver Synchronous authorized presentation receiver.
  /// \return Bounded row counts and forward continuation.
  [[nodiscard]] PageWindowState
  Open_Window(PageOpenMode mode, std::size_t limit, PageWindowReceiver &receiver) {
    Require_();
    ValidateRecordWindowLimit(limit);
    if (mode != PageOpenMode::View && mode != PageOpenMode::Edit) {
      throw Error("Unsupported list opening mode.", "PageWindowMode");
    }
    PageWindowState result;
    if (Base::IsOpen()) {
      throw Error(PageSessionDiagnostics::kAlreadyOpen, PageSessionDiagnostics::kAlreadyOpenCode);
    }
    window_.reset();
    selected_ = {};
    Base::OpenWith_(false, [&](P &page) {
      static_cast<void>(detail::InitializePage(page, mode == PageOpenMode::Edit, false));
      result = Load_(RecordWindowPosition::First, limit, receiver, true);
    });
    return result;
  }

  /// \brief Discards display boundaries after explicit single-record navigation.
  void Forget_Window() {
    window_.reset();
    selected_ = {};
  }

  /// \return Original selected SQL identity, not a key changed only in an AL buffer.
  [[nodiscard]] RecordId Window_Current_Record() const {
    static_cast<void>(Base::Page_());
    return window_.has_value() ? selected_ : Base::Current_Record();
  }

  /// \brief Selects an exact retained SQL row while retaining the loaded block's boundaries.
  /// \param identity Server-retained SQL identity.
  /// \return Whether the row still exists.
  [[nodiscard]] bool Select_Window_Record(const RecordId &identity) {
    Require_();
    static_cast<void>(Base::Page_());
    if (!window_.has_value()) { throw Error("The page has no loaded block.", "PageWindowAnchor"); }
    if constexpr (Base::kHasRecord) {
      using Source = std::remove_cvref_t<decltype(Base::Record_())>;
      Source source;
      bool retained = false;
      for (std::size_t i = 0; i < window_->Size(); ++i) {
        window_->Load(i, &source);
        if (source.RecordId() == identity) {
          retained = true;
          break;
        }
      }
      if (!retained) { throw Error("Selected row is outside the window.", "PageWindowSelection"); }
      try {
        if (!Base::Select_Record(identity)) { return false; }
        selected_ = identity;
        return true;
      } catch (...) {
        Forget_Window();
        Base::Release_();
        throw;
      }
    }
    return false;
  }

  /// \brief Loads a block relative to retained SQL boundaries, not mutated AL key values.
  /// \param position Requested SQL-order movement.
  /// \param limit Positive trusted server bound.
  /// \param receiver Synchronous authorized presentation receiver.
  /// \return Bounded rows and continuation in the requested direction.
  [[nodiscard]] PageWindowState
  Read_Window(PageWindowPosition position, std::size_t limit, PageWindowReceiver &receiver) {
    Require_();
    ValidateRecordWindowLimit(limit);
    static_cast<void>(Base::Page_());
    const RecordWindowPosition sqlPosition = Position_(position);
    if ((sqlPosition == RecordWindowPosition::After ||
         sqlPosition == RecordWindowPosition::Before) &&
        (!window_.has_value() || window_->Size() == 0)) {
      throw Error("List continuation has no SQL boundary.", "PageWindowAnchor");
    }
    try {
      Base::RowLeft();
      return Load_(sqlPosition, limit, receiver, false);
    } catch (...) {
      window_.reset();
      Base::Release_();
      throw;
    }
  }

private:
  static void Require_() {
    if constexpr (
        !Base::kHasRecord || PageTraits<P>::kPage.type != PageType::List ||
        requires { &P::OnFindRecord; } || requires { &P::OnNextRecord; }) {
      throw Error("This page has no qualified SQL list provider.", "PageWindowProvider");
    }
  }

  static RecordWindowPosition Position_(PageWindowPosition position) {
    switch (position) {
      case PageWindowPosition::First: return RecordWindowPosition::First;
      case PageWindowPosition::Next: return RecordWindowPosition::After;
      case PageWindowPosition::Previous: return RecordWindowPosition::Before;
      case PageWindowPosition::Last: return RecordWindowPosition::Last;
      case PageWindowPosition::Unknown: break;
    }
    throw Error("Unknown list window movement.", "PageWindowPosition");
  }

  PageWindowState Load_(RecordWindowPosition position,
                        std::size_t limit,
                        PageWindowReceiver &receiver,
                        bool opening) {
    if constexpr (!Base::kHasRecord) {
      static_cast<void>(position);
      static_cast<void>(limit);
      static_cast<void>(receiver);
      static_cast<void>(opening);
      throw Error("The page has no source record.", "PageWindowProvider");
    } else {
      return Load_Record_Window_(position, limit, receiver, opening);
    }
  }

  PageWindowState Load_Record_Window_(RecordWindowPosition position,
                                      std::size_t limit,
                                      PageWindowReceiver &receiver,
                                      bool opening) {
    auto &page = Base::Page_();
    auto &rec = page.Rec;
    using Source = std::remove_cvref_t<decltype(rec)>;
    Source anchor;
    static_cast<typename Source::Platform_Half &>(anchor).Copy(rec);
    if (position == RecordWindowPosition::After || position == RecordWindowPosition::Before) {
      window_->Load(position == RecordWindowPosition::After ? window_->Size() - 1 : 0, &anchor);
    }
    auto loaded = ReadRecordWindow(&anchor, TableTraits<Source>::kTable, position, limit);
    if (loaded.Size() == 0 && opening && page.OpenedEditable()) {
      throw Error("Empty editable list windows are not qualified.", "PageWindowNewRow");
    }
    if (loaded.Size() == 0 &&
        (position == RecordWindowPosition::After || position == RecordWindowPosition::Before)) {
      receiver.Current(selected_, *this);
      return {0, loaded.RowsRead(), false};
    }
    const RecordId previous = window_.has_value() ? selected_ : RecordId{};
    RecordId selected;
    Source selectedValues;
    detail::HeldImage selectedImage;
    for (std::size_t i = 0; i < loaded.Size(); ++i) {
      loaded.Load(i, &rec);
      const RecordId identity = rec.RecordId();
      static_cast<void>(static_cast<typename Source::Platform_Half &>(rec).Read(true));
      page.LandedOnRecord();
      detail::AfterReadPageRecord(page);
      receiver.Row(identity, *this);
      if (i == 0 || identity == previous) {
        selected = identity;
        selectedValues = rec;
        selectedImage.SetFrom(reinterpret_cast<detail::StateHandle *>(&rec)->Ensure().image);
      }
    }
    if (loaded.Size() != 0) {
      rec = selectedValues;
      reinterpret_cast<detail::StateHandle *>(&rec)->Ensure().image.SetFrom(selectedImage);
    } else {
      rec = Source{};
      reinterpret_cast<detail::StateHandle *>(&rec)->Ensure().positioned = false;
    }
    if (opening || selected != previous) { detail::AfterCurrentPageRecord(page); }
    receiver.Current(selected, *this);
    const PageWindowState result{loaded.Size(), loaded.RowsRead(), loaded.HasMore()};
    window_ = std::move(loaded);
    selected_ = selected;
    return result;
  }

  std::optional<RecordWindow> window_;
  RecordId selected_;
};

/// \brief Makes the closed production adapter for a generated page catalogue entry.
/// \tparam P The generated page class.
/// \return An owned PageInstance; no TestPage controls, traps or AL opening are created.
template <typename P> PageInstance *MakePageSession() {
  class Adapter final : public PageInstance {
  public:
    [[nodiscard]] PageCore &Controls() override { return session_; }

    [[nodiscard]] const PageDef &Declaration() const override { return PageTraits<P>::kPage; }

    void PrepareBorrowed(void *object, PageId identity) override {
      if (object == nullptr || identity != PageTraits<P>::kId) {
        throw Error("Invalid borrowed page identity.", "UiModalIdentity");
      }
      session_.Prepare_Borrowed(*static_cast<P *>(object));
    }

    [[nodiscard]] Action CloseModal(Action action) override {
      const auto result = session_.Close_Modal(action);
      session_.Forget_Window();
      return result;
    }

    void Open(PageOpenMode mode) override {
      switch (mode) {
        case PageOpenMode::View: session_.OpenView(); return;
        case PageOpenMode::Edit: session_.OpenEdit(); return;
        case PageOpenMode::New: session_.OpenNew(); return;
        case PageOpenMode::Unknown: break;
      }
      throw Error("Unknown page opening mode.", "PageOpenMode");
    }

    void Close() override {
      session_.Close();
      session_.Forget_Window();
    }

    void Save() override { session_.RowLeft(); }

    [[nodiscard]] bool IsOpen() const override { return session_.IsOpen(); }

    [[nodiscard]] bool Move(PagePosition position) override {
      if (position == PagePosition::Unknown) {
        throw Error("Unknown page movement.", "PagePosition");
      }
      session_.Forget_Window();
      switch (position) {
        case PagePosition::First: return session_.First();
        case PagePosition::Next: return session_.Next();
        case PagePosition::Previous: return session_.Previous();
        case PagePosition::Last: return session_.Last();
        case PagePosition::Unknown: break;
      }
      throw Error("Unknown page movement.", "PagePosition");
    }

    [[nodiscard]] bool SelectRecord(const RecordId &record) override {
      session_.Forget_Window();
      return session_.Select_Record(record);
    }

    [[nodiscard]] RecordId CurrentRecord() const override {
      return session_.Window_Current_Record();
    }

    [[nodiscard]] PageWindowState
    OpenWindow(PageOpenMode mode, std::size_t limit, PageWindowReceiver &receiver) override {
      return session_.Open_Window(mode, limit, receiver);
    }

    [[nodiscard]] PageWindowState ReadWindow(PageWindowPosition position,
                                             std::size_t limit,
                                             PageWindowReceiver &receiver) override {
      return session_.Read_Window(position, limit, receiver);
    }

    [[nodiscard]] bool SelectWindowRecord(const RecordId &record) override {
      return session_.Select_Window_Record(record);
    }

  private:
    PageWindowSession<P> session_;
  };

  return new Adapter();
}

}
