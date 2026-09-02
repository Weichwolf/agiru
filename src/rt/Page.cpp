#include "runtime/Page.h"

#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include <string>
#include <string_view>

namespace agiru {

void TestField::Unbound() const {
  throw Error("the control '" + std::string(name_) + "' is not on a running page (board:0030)");
}

void TestField::SetValue(std::string_view value) {
  static_cast<void>(value);
  Unbound();
}

std::string TestField::Value() const {
  Unbound();
}

void TestField::AssertEquals(std::string_view expected) const {
  static_cast<void>(expected);
  Unbound();
}

Integer TestField::AsInteger() const {
  Unbound();
}

Boolean TestField::AsBoolean() const {
  Unbound();
}

void TestField::Activate() {
  Unbound();
}

void TestField::Lookup() {
  Unbound();
}

void TestField::DrillDown() {
  Unbound();
}

Boolean TestField::Editable() const {
  Unbound();
}

Boolean TestField::Enabled() const {
  Unbound();
}

Boolean TestField::Visible() const {
  Unbound();
}

void TestAction::Unbound() const {
  throw Error("the action '" + std::string(name_) + "' is not on a running page (board:0030)");
}

void TestAction::Invoke() {
  Unbound();
}

Boolean TestAction::Enabled() const {
  Unbound();
}

Boolean TestAction::Visible() const {
  Unbound();
}

} // namespace agiru
