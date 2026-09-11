#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/test/Handlers.h"
#include "runtime/test/TestPermissions.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Notification.h"
#include "type/Text.h"
#include "type/TransactionModel.h"

#include <concepts>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

/// \file
/// \brief AL's test runner -- the `[Test]` procedures of a `Subtype = Test` codeunit.

namespace agiru {

/// \brief One `[Test]` procedure.
struct TestMethod {
  std::string_view name;                 ///< The procedure's AL name.
  void (*invoke)(void *instance);        ///< Calls the procedure on the codeunit run's instance.
  std::optional<TransactionModel> model; ///< Its `[TransactionModel]`, empty when it declares none.
  std::span<const std::string_view> handlers; ///< The names its `[HandlerFunctions]` listed, in
                                              ///< the order AL wrote them. A named handler that
                                              ///< never ran fails the case (board:0054).
  ::agiru::TestPermissions permissions =
      ::agiru::TestPermissions::Restrictive; ///< Its `[TestPermissions]`, RESOLVED: a method that
                                             ///< inherits carries its codeunit's value, and a
                                             ///< codeunit that declares none carries `Restrictive`,
                                             ///< which is AL's own default
                                             ///< (`devenv-testing-with-permission-sets.md`).
                                             ///< \warning THE SENTINEL NEVER TRAVELS. AL hands the
                                             ///< value to `OnBeforeTestRun`, and the 8 907
                                             ///< `LibraryLowerPermissions` call sites resolve
                                             ///< nothing (board:0224).
                                             ///< \warning EMPTY IS NOT `AutoRollback`. A declared
                                             ///< `AutoRollback` discards the method's writes, as
                                             ///< `attributes/devenv-transactionmodel-attribute.md`
  ///< states; an ABSENT attribute leaves the decision to the
  ///< runner's `TestIsolation`, and under `Codeunit` a
  ///< passing method's writes reach the next one.
};

/// \brief The test codeunit instance the runner is driving, for the handler thunks.
/// \return The instance, or `nullptr` outside a codeunit run.
[[nodiscard]] void *CurrentTestInstance();

/// \brief Sets the instance `CurrentTestInstance` answers.
/// \param instance The instance, or `nullptr` when the run is over.
void SetCurrentTestInstance(void *instance);

/// \brief Makes a test codeunit for one codeunit run.
/// \tparam Codeunit The generated codeunit class.
/// \return The instance, owned by the caller.
template <typename Codeunit> void *MakeTestCodeunit() {
  return new Codeunit{};
}

/// \brief Unmakes what `MakeTestCodeunit` made.
/// \tparam Codeunit The generated codeunit class.
/// \param instance The instance.
template <typename Codeunit> void FreeTestCodeunit(void *instance) {
  delete static_cast<Codeunit *>(instance);
}

/// \brief Calls one `[Test]` procedure, or `OnRun`, on the codeunit run's instance.
///
/// \tparam Codeunit The generated codeunit class.
/// \tparam Method   The procedure.
/// \param instance The instance `MakeTestCodeunit` made for this codeunit run.
///
/// \note ONE INSTANCE PER CODEUNIT RUN, which is what `devenv-test-codeunits-and-test-methods.md`
///       describes: "When a test codeunit runs, it runs the OnRun trigger, and then runs each
///       test method in the codeunit." The globals live for the run -- 40 of the 78 UT
///       codeunits keep an `IsInitialized` there and bind a `Manual` subscriber once under it --
///       while each method still has its own TRANSACTION. A fresh object per method bound that
///       subscriber once per test and left the earlier bindings dangling (measured 2026-09-09:
///       24 cases reading "multiple subscribers competing").
template <typename Codeunit, void (Codeunit::*Method)()> void InvokeTest(void *instance) {
  (static_cast<Codeunit *>(instance)->*Method)();
}

/// \brief Calls one HANDLER procedure on a freshly made codeunit, with whatever the dialog hands
///        it.
/// \tparam Codeunit The test codeunit's class.
/// \tparam Method   The handler procedure.
/// \tparam Arguments What the dialog passes -- the question and the reply, the text, the page.
/// \param arguments The dialog's own arguments.
///
/// \note THE THUNK IS THE ONLY PLACE THE SIGNATURE IS KNOWN. `TestHandler::invoke` is a `void *`
///       because AL's thirteen handler kinds have thirteen signatures; the caller casts it back to
///       the one its kind states, and this template is what it points at (board:0054).
namespace detail {

/// \brief The first parameter of a handler method, for a `[PageHandler]`'s `var TestPage`.
/// \tparam C The codeunit.
/// \tparam A The parameter's type.
/// \return Never called; it exists for `decltype`.
template <typename C, typename A> A &FirstParameterOf(void (C::*)(A &));

}

/// \brief What a `[FilterPageHandler]` is handed: the filter page's first control as the
///        RecordRef the procedure declares `var`, and where its Boolean answer goes.
struct FilterPageAnswer {
  ::agiru::RecordRef &record; ///< The control's record, whose filters the handler edits.
  ::agiru::Boolean accepted;  ///< What the handler returned -- whether the user pressed OK.
};

template <typename Codeunit, auto Method> void InvokeHandler(std::string_view text, void *reply) {
  std::optional<Codeunit> own;
  if (CurrentTestInstance() == nullptr) { own.emplace(); }
  Codeunit &codeunit = own.has_value() ? *own : *static_cast<Codeunit *>(CurrentTestInstance());
  if constexpr (requires {
                  typename std::remove_cvref_t<decltype(detail::FirstParameterOf(
                      Method))>::IsTestPage;
                }) {
    static_cast<void>(text);
    using Harness = std::remove_cvref_t<decltype(detail::FirstParameterOf(Method))>;
    Harness harness;
    harness.Adopt(reply);
    (codeunit.*Method)(harness);
  } else if constexpr (requires { (codeunit.*Method)(); }) {
    static_cast<void>(text);
    static_cast<void>(reply);
    (codeunit.*Method)();
  } else if constexpr (requires { (codeunit.*Method)(::agiru::Text<0>{}); }) {
    static_cast<void>(reply);
    (codeunit.*Method)(::agiru::Text<0>{text});
  } else if constexpr (requires(::agiru::Boolean answer) {
                         (codeunit.*Method)(::agiru::Text<0>{}, answer);
                       }) {
    (codeunit.*Method)(::agiru::Text<0>{text}, *static_cast<::agiru::Boolean *>(reply));
  } else if constexpr (requires(::agiru::Integer answer) {
                         (codeunit.*Method)(::agiru::Text<0>{}, answer);
                       }) {
    (codeunit.*Method)(::agiru::Text<0>{text}, *static_cast<::agiru::Integer *>(reply));
  } else if constexpr (requires {
                         typename std::remove_cvref_t<decltype(detail::FirstParameterOf(
                             Method))>::IsReport;
                       }) {
    static_cast<void>(text);
    using Handled = std::remove_cvref_t<decltype(detail::FirstParameterOf(Method))>;
    (codeunit.*Method)(*static_cast<Handled *>(reply));
  } else if constexpr (requires(::agiru::Notification &sent) { (codeunit.*Method)(sent); }) {
    static_cast<void>(text);
    static_cast<void>((codeunit.*Method)(*static_cast<::agiru::Notification *>(reply)));
  } else if constexpr (requires(::agiru::RecordRef &record) {
                         { (codeunit.*Method)(record) } -> std::convertible_to<::agiru::Boolean>;
                       }) {
    static_cast<void>(text);
    auto *answer = static_cast<FilterPageAnswer *>(reply);
    answer->accepted = (codeunit.*Method)(answer->record);
  } else {
    static_cast<void>(text);
    static_cast<void>(reply);
    throw Error("A handler of this shape needs a running UI (board:0030)");
  }
}

/// \brief One `Subtype = Test` codeunit, as the runner sees it.
///
/// \note IT REGISTERS ITSELF FROM THE GENERATED SOURCE. The alternative is a catalogue file the
///       transpiler assembles for the whole run, which would make every app's translation depend on
///       every other app's. What that costs is the ORDER: static initialisation across translation
///       units has none, so the runner sorts by codeunit number before it runs anything.
class TestCatalogue {
public:
  /// \brief Registers a test codeunit.
  ///
  /// \param id      The codeunit's AL number.
  /// \param name    Its AL name.
  /// \param make    Makes the instance one codeunit run drives (`MakeTestCodeunit`).
  /// \param free    Unmakes it.
  /// \param onRun   Its `OnRun` trigger, which AL runs before the test procedures.
  /// \param methods Its `[Test]` procedures, in declaration order.
  /// \param handlers Its handler procedures.
  TestCatalogue(CodeunitId id,
                std::string_view name,
                void *(*make)(),
                void (*free)(void *),
                void (*onRun)(void *),
                std::span<const TestMethod> methods,
                std::span<const TestHandler> handlers = {});

  TestCatalogue(const TestCatalogue &) = delete;
  TestCatalogue(TestCatalogue &&) = delete;
  TestCatalogue &operator=(const TestCatalogue &) = delete;
  TestCatalogue &operator=(TestCatalogue &&) = delete;
  ~TestCatalogue() = default;

  /// \brief The codeunit's AL number.
  /// \return The number.
  [[nodiscard]] CodeunitId Id() const { return id_; }

  /// \brief The codeunit's AL name.
  /// \return The name.
  [[nodiscard]] std::string_view Name() const { return name_; }

  /// \brief The codeunit's `OnRun` trigger.
  /// \return What calls it.
  [[nodiscard]] auto OnRun() const { return onRun_; }

  /// \brief Makes the instance for one codeunit run.
  /// \return The instance; `Free` unmakes it.
  [[nodiscard]] void *Make() const { return make_(); }

  /// \brief Unmakes what `Make` made.
  /// \param instance The instance.
  void Free(void *instance) const { free_(instance); }

  /// \brief The `[Test]` procedures.
  /// \return Them, in declaration order.
  [[nodiscard]] std::span<const TestMethod> Methods() const { return methods_; }

  /// \brief The codeunit's handler procedures, whichever cases name them.
  /// \return The table, empty when the codeunit declares none.
  [[nodiscard]] std::span<const TestHandler> Handlers() const { return handlers_; }

private:
  CodeunitId id_;
  std::string_view name_;
  void *(*make_)();
  void (*free_)(void *);
  void (*onRun_)(void *);
  std::span<const TestMethod> methods_;
  std::span<const TestHandler> handlers_;
};

/// \brief What one `[Test]` procedure did.
struct TestResult {
  std::string_view codeunit; ///< The codeunit's AL name.
  std::string_view method;   ///< The procedure's AL name.
  bool passed;               ///< True when nothing was raised.
  std::string error;         ///< What was raised, when something was.
};

/// \brief What a run did.
struct TestRun {
  std::vector<TestResult> results; ///< Every procedure that ran, in the order it ran.
  std::size_t passed = 0;          ///< How many did not raise.
  std::size_t failed = 0;          ///< How many did.
};

/// \brief Runs the registered `[Test]` procedures.
///
/// \param codeunit The AL name of one test codeunit, or empty for all of them.
/// \return What each procedure did.
///
/// \note THE ORDER IS THE CODEUNIT NUMBER AND THEN THE DECLARATION, never the order the catalogues
///       registered in -- that one is the linker's and would make a run unrepeatable.
/// \warning A FAILING PROCEDURE DOES NOT STOP THE NEXT ONE.
/// `devenv-test-codeunits-and-test-methods.md`
///          separates a test codeunit from a normal one on exactly this.
[[nodiscard]] TestRun RunRegisteredTests(std::string_view codeunit);

/// \brief What a run reports about one procedure the moment it finishes.
using TestReport = void (*)(const TestResult &);

/// \brief Runs the registered `[Test]` procedures, reporting each one as it finishes.
///
/// \param codeunit The AL name of one test codeunit, or empty for all of them.
/// \param report   Called after every procedure, before the next one starts.
/// \return What each procedure did.
///
/// \note A RUN THAT PRINTS ONLY AT THE END LOSES EVERYTHING IT DID when it does not reach the
///       end. A run over the whole population is minutes and 855 codeunits, and a hard failure
///       inside one of them -- a segmentation fault, which no `catch` sees -- took the entire
///       report with it: 0 lines of output for a run that had done most of its work. Reporting as
///       it goes is what makes such a run say where it stopped.
[[nodiscard]] TestRun RunRegisteredTests(std::string_view codeunit, TestReport report);

/// \brief Every registered test codeunit.
/// \return Them, by codeunit number.
[[nodiscard]] std::vector<const TestCatalogue *> RegisteredTestCodeunits();

}
