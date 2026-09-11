#include "runtime/test/TestPage.h"

#include "runtime/Error.h"
#include "runtime/test/PageCore.h"
#include "runtime/test/TestAction.h"
#include "runtime/test/TestField.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/Decimal.h"
#include "type/Integer.h"

#include "BuiltinsWritten.h"

#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

void TestField::Unbound() const {
  throw Error("the control '" + std::string(name_) + "' is not on a running page (board:0030)");
}

void TestField::SetValueText(std::string_view value) {
  if (core_ == nullptr) { Unbound(); }
  core_->SetControlText(name_, value);
}

std::string TestField::Value() const {
  if (core_ == nullptr) { Unbound(); }
  return core_->ControlText(name_);
}

void TestField::AssertEqualsText(std::string_view expected) const {
  const std::string actual = Value();
  if (actual == expected) { return; }
  Decimal left{};
  Decimal right{};
  if (detail::Evaluated(left, actual) && detail::Evaluated(right, expected) && left == right) {
    return;
  }
  const std::string ordinal = core_->ControlOrdinal(name_);
  if (!ordinal.empty() && ordinal == expected) { return; }
  throw Error("Assert.AreEqual failed. Expected:<" + std::string(expected) + ">. Actual:<" +
              actual + ">. Control '" + std::string(name_) + "'.");
}

Integer TestField::AsInteger() const {
  Integer value{};
  if (!detail::Evaluated(value, Value())) {
    throw Error("the control '" + std::string(name_) + "' does not hold an Integer");
  }
  return value;
}

Boolean TestField::AsBoolean() const {
  Boolean value{};
  if (!detail::Evaluated(value, Value())) {
    throw Error("the control '" + std::string(name_) + "' does not hold a Boolean");
  }
  return value;
}

Decimal TestField::AsDecimal() const {
  Decimal value{};
  if (!detail::Evaluated(value, Value())) {
    throw Error("the control '" + std::string(name_) + "' does not hold a Decimal");
  }
  return value;
}

Date TestField::AsDate() const {
  Date value{};
  if (!detail::Evaluated(value, Value())) {
    throw Error("the control '" + std::string(name_) + "' does not hold a Date");
  }
  return value;
}

void TestField::Activate() {
  if (core_ == nullptr) { Unbound(); }
}

void TestField::Lookup() {
  if (core_ == nullptr) { Unbound(); }
  core_->RunControlTrigger(name_, ControlTriggerKind::Lookup);
}

void TestField::DrillDown() {
  if (core_ == nullptr) { Unbound(); }
  core_->RunControlTrigger(name_, ControlTriggerKind::DrillDown);
}

void TestField::Drilldown() const {
  if (core_ == nullptr) { Unbound(); }
  core_->RunControlTrigger(name_, ControlTriggerKind::DrillDown);
}

void TestField::AssistEdit() const {
  if (core_ == nullptr) { Unbound(); }
  core_->RunControlTrigger(name_, ControlTriggerKind::AssistEdit);
}

std::string TestField::Caption() const {
  if (core_ == nullptr) { Unbound(); }
  return core_->ControlCaption(name_);
}

Boolean TestField::Editable() const {
  if (core_ == nullptr) { Unbound(); }
  return core_->ControlEditable(name_);
}

Boolean TestField::Enabled() const {
  if (core_ == nullptr) { Unbound(); }
  return core_->ControlEnabled(name_);
}

Boolean TestField::Visible() const {
  if (core_ == nullptr) { Unbound(); }
  return core_->ControlVisible(name_);
}

void TestAction::Unbound() const {
  throw Error("the action '" + std::string(name_) + "' is not on a running page (board:0030)");
}

void TestAction::Invoke() {
  if (core_ == nullptr) { Unbound(); }
  core_->RunControlTrigger(name_, ControlTriggerKind::Action);
}

Boolean TestAction::Enabled() const {
  if (core_ == nullptr) { Unbound(); }
  return core_->ControlEnabled(name_);
}

Boolean TestAction::Visible() const {
  if (core_ == nullptr) { Unbound(); }
  return core_->ControlVisible(name_);
}

namespace detail {

namespace {

struct PageTrap {
  std::int32_t page;
  void *harness;
  void (*adopt)(void *harness, void *page, bool owned);
};

std::vector<PageTrap> &Traps() {
  static thread_local std::vector<PageTrap> traps;
  return traps;
}

}

void TrapPage(std::int32_t page,
              void *harness,
              void (*adopt)(void *harness, void *page, bool owned)) {
  std::erase_if(Traps(), [harness](const PageTrap &trap) { return trap.harness == harness; });
  Traps().push_back({.page = page, .harness = harness, .adopt = adopt});
}

bool ReleaseTrap(std::int32_t page, void *object, bool owned) {
  auto &traps = Traps();
  for (auto it = traps.rbegin(); it != traps.rend(); ++it) {
    if (it->page != page) { continue; }
    const PageTrap trap = *it;
    traps.erase(std::next(it).base());
    trap.adopt(trap.harness, object, owned);
    return true;
  }
  return false;
}

bool TrapPending(std::int32_t page) {
  return std::ranges::any_of(Traps(), [page](const PageTrap &trap) { return trap.page == page; });
}

void ClearTraps() {
  Traps().clear();
}

}

}
