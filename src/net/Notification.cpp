#include "type/Notification.h"

#include "type/Boolean.h"
#include "type/Guid.h"
#include "type/Integer.h"

#include <string>
#include <string_view>

namespace agiru {

std::string Notification::GetData(std::string_view key) const {
  const std::string named{key};
  return data_.ContainsKey(named) ? data_.Get(named) : std::string{};
}

void Notification::AddAction(std::string_view caption,
                             Integer codeunitId,
                             std::string_view methodName) {
  SetData("Action" + std::to_string(actions_),
          std::string(caption) + "|" + std::to_string(codeunitId) + "|" + std::string(methodName));
  ++actions_;
}

::agiru::Boolean Notification::Send() {
  if (id_.IsNull()) { id_ = Guid::Create(); }
  if (Sink_() != nullptr) { Sink_()(*this); }
  return true;
}

Notification::Sink &Notification::Sink_() {
  static Sink sink = nullptr;
  return sink;
}

void Notification::OnSend(Sink sink) {
  Sink_() = sink;
}

::agiru::Boolean Notification::Recall() {
  return true;
}

}
