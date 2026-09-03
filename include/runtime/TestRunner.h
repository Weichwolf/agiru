#pragma once

#include "meta/Ids.h"
#include "type/TransactionModel.h"

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

/// \file
/// \brief AL's test runner -- the `[Test]` procedures of a `Subtype = Test` codeunit.

namespace agiru {

/// \brief One `[Test]` procedure.
struct TestMethod {
  std::string_view name;                 ///< The procedure's AL name.
  void (*invoke)();                      ///< Makes the codeunit and calls the procedure.
  std::optional<TransactionModel> model; ///< Its `[TransactionModel]`, empty when it declares none.
                                         ///< \warning EMPTY IS NOT `AutoRollback`. A declared
                                         ///< `AutoRollback` discards the method's writes, as
                                         ///< `attributes/devenv-transactionmodel-attribute.md`
                                         ///< states; an ABSENT attribute leaves the decision to the
                                         ///< runner's `TestIsolation`, and under `Codeunit` a
                                         ///< passing method's writes reach the next one.
};

/// \brief Calls one `[Test]` procedure on a freshly made codeunit.
///
/// \tparam Codeunit The generated codeunit class.
/// \tparam Method   The procedure.
///
/// \note A TEST GETS ITS OWN OBJECT, which is what "each test method runs in a separate database
///       transaction" means for the object beside the transaction: a global left over from the
///       previous test would make the outcome depend on the order.
template <typename Codeunit, void (Codeunit::*Method)()> void InvokeTest() {
  Codeunit codeunit{};
  (codeunit.*Method)();
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
  /// \param onRun   Its `OnRun` trigger, which AL runs before the test procedures.
  /// \param methods Its `[Test]` procedures, in declaration order.
  TestCatalogue(CodeunitId id,
                std::string_view name,
                void (*onRun)(),
                std::span<const TestMethod> methods);

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

  /// \brief The `[Test]` procedures.
  /// \return Them, in declaration order.
  [[nodiscard]] std::span<const TestMethod> Methods() const { return methods_; }

private:
  CodeunitId id_;
  std::string_view name_;
  void (*onRun_)();
  std::span<const TestMethod> methods_;
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

/// \brief Every registered test codeunit.
/// \return Them, by codeunit number.
[[nodiscard]] std::vector<const TestCatalogue *> RegisteredTestCodeunits();

}
