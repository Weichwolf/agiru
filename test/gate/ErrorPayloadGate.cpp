#include "runtime/ErrorValue.h"

#include "Check.h"

#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

void MessagesAndCodesAreOwned() {
  std::string text = "Posting failed";
  std::string code = "TestField";
  agiru::Error error{std::string_view(text), std::string_view(code)};
  text = "Changed";
  code = "Changed";
  CHECK_TEXT("message survives source mutation", error.what(), "Posting failed");
  CHECK_TEXT("code survives source mutation", error.Code(), "TestField");
  const agiru::Error copied = error;
  error = agiru::Error{"Changed", "Dialog"};
  CHECK_TEXT("copy keeps the message", copied.what(), "Posting failed");
  CHECK_TEXT("copy keeps the code", copied.Code(), "TestField");
  agiru::Error movable = copied;
  const agiru::Error moved = std::move(movable);
  CHECK_TEXT("move keeps the message", moved.what(), "Posting failed");
  CHECK_TEXT("move keeps the code", moved.Code(), "TestField");
  const agiru::Error literal{"Literal"};
  CHECK_TEXT("inherited constructor remains available", literal.what(), "Literal");
  CHECK_TEXT("no classification default in the value layer", literal.Code(), "");
}

void CodingDoesNotReinterpretAnExistingError() {
  const agiru::Error plain{std::string_view{"Missing"}};
  const agiru::Error coded = plain.Coded("DB:RecordNotFound");
  CHECK_TEXT("coding preserves the text", coded.what(), "Missing");
  CHECK_TEXT("coding fills an empty code", coded.Code(), "DB:RecordNotFound");
  CHECK_TEXT("coding does not mutate the original", plain.Code(), "");
  CHECK_TEXT("an existing code wins", coded.Coded("Dialog").Code(), "DB:RecordNotFound");
}

void InfoAndNonAlErrorsRemainDistinct() {
  struct Description {
    [[nodiscard]] static std::string_view Message() { return "Described"; }
  };

  const agiru::Error described{Description{}};
  CHECK_TEXT("ErrorInfo-compatible constructor keeps the message", described.what(), "Described");
  bool runtimeCaught = false;
  try {
    throw agiru::Error(Description{});
  } catch (const std::runtime_error &) { runtimeCaught = true; }
  CHECK_TRUE("the standard exception base is unchanged", runtimeCaught);
  bool incorrectlyCaught = false;
  bool allocationCaught = false;
  try {
    throw std::bad_alloc{};
  } catch (const agiru::Error &) { incorrectlyCaught = true; } catch (const std::bad_alloc &) {
    allocationCaught = true;
  }
  CHECK_TRUE("allocation failures are not AL errors", allocationCaught && !incorrectlyCaught);
}

}

int main() {
  return gate::Run("ErrorPayload", [] {
    MessagesAndCodesAreOwned();
    CodingDoesNotReinterpretAnExistingError();
    InfoAndNonAlErrorsRemainDistinct();
  });
}
