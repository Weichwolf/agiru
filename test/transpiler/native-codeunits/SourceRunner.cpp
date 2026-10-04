#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/ErrorValue.h"
#include "type/Integer.h"

#include "Check.h"
#include "PlatformModule.h"
#include "fixture/codeunit/NativeConsumer.h"
#include "system/fixture/codeunit/SourceNative.h"

#include <string>

namespace {

using Native = agiru::System::Fixture::SourceNative_Codeunit;
constexpr agiru::CodeunitId kNativeId{50321};
constexpr agiru::CodeunitId kPeerId{50322};
constexpr agiru::Integer kValue = 7;

static_assert(agiru::CodeunitTraits<Native>::kId == kNativeId);
static_assert(agiru::app::Platform::kModule.id == "85a884cd-20d8-4d18-91bd-e6c1baaa3a32");
static_assert(agiru::app::Platform::kModule.version == "1.2.3.4");

void SourceOwnedNativeCodeunits() {
  agiru::Fixture::NativeConsumer_Codeunit consumer;
  CHECK_TRUE("bare native source name binds through production indexing",
             consumer.ByName() == kValue);
  CHECK_TRUE("qualified native source name binds through production indexing",
             consumer.ByNamespace() == kValue);
  CHECK_TRUE("native source ID binds through production indexing", consumer.ById() == kValue);
  agiru::Integer value = kValue;
  std::string error;
  try {
    static_cast<void>(consumer.Unbound(value));
  } catch (const agiru::Error &failure) { error = failure.what(); }
  CHECK_TRUE(
      "source-bound Native call retains its typed identity",
      error.contains("codeunit 50321 System.Fixture.Source Native.Read(var Value: Integer): Text"));
  CHECK_TRUE("source-bound Native call refuses before AL effects", value == kValue);
  const auto *entry = agiru::FindCodeunit(kNativeId);
  CHECK_TRUE("native declaration registers under its original identity",
             entry != nullptr && entry->name == "Source Native");
  const auto *peer = agiru::FindCodeunit(kPeerId);
  CHECK_TRUE("native source dependencies also register under their original identity",
             peer != nullptr && peer->name == "Source Peer");
  CHECK_TRUE("native metadata preserves the source declaration",
             agiru::CodeunitTraits<Native>::kCodeunit.id == kNativeId &&
                 agiru::CodeunitTraits<Native>::kCodeunit.name == "Source Native");
}

}

int main() {
  return gate::Run("Source-Owned Native Codeunits", SourceOwnedNativeCodeunits);
}
