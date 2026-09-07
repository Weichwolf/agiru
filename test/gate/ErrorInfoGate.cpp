#include "runtime/Error.h"
#include "type/ErrorInfo.h"

#include "Check.h"

#include <string>

using agiru::Error;
using agiru::ErrorInfo;

namespace {

// errorinfo-create-method.md: `ErrorInfo.Create(Message [, Collectible] ...)`;
// errorinfo-create--method.md: "Creates a new ErrorInfo object with Collectible set to true."
void CreateCarriesWhatItWasGiven() {
  ErrorInfo info = ErrorInfo::Create("Posting date is missing", true);
  CHECK_TEXT("the message is the one given", info.Message(), "Posting date is missing");
  CHECK_TRUE("and so is collectibility", info.Collectible());
  CHECK_TRUE("the one-argument form is not collectible", !ErrorInfo::Create("x").Collectible());
  CHECK_TRUE("the bare form is", ErrorInfo::Create().Collectible());
}

// error-method.md: `Error(ErrorInfo)` raises the error the info describes.
void ErrorRaisesTheMessage() {
  std::string raised;
  try {
    throw Error(ErrorInfo::Create("Amount must be positive"));
  } catch (const Error &error) { raised = error.what(); }
  CHECK_TEXT("the thrown text is the info's message", raised, "Amount must be positive");
}

// errorinfo-message-method.md: `[NewMessage := ] ErrorInfo.Message([NewMessage])` -- the setter
// returns what it set and the getter reads it back.
void ASetterReturnsAndKeeps() {
  ErrorInfo info = ErrorInfo::Create();
  CHECK_TEXT("the setter returns the value", info.Title("Title"), "Title");
  CHECK_TEXT("and the getter reads it back", info.Title(), "Title");
  CHECK_TRUE("an integer setter likewise", info.FieldNo(7) == 7 && info.FieldNo() == 7);
}

} // namespace

int main() {
  return gate::Run("ErrorInfo", [] {
    CreateCarriesWhatItWasGiven();
    ErrorRaisesTheMessage();
    ASetterReturnsAndKeeps();
  });
}
