#include "dotnet/XmlReader.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Transaction.h"
#include "type/NumberSequence.h"

#include <algorithm>
#include <array>
#include <barrier>
#include <cstdio>
#include <exception>
#include <string>
#include <thread>

namespace {
constexpr auto kDsn = "postgresql://agiru:agiru@localhost:5433/agiru_gate";

int XmlPolicy() {
  agiru::dotnet::StringReader input;
  input = input.StringReader("<!DOCTYPE root [<!ENTITY own SYSTEM 'file:///home/cosmo/Git/agiru/build/review-20260928/xml-owned-fixture.txt'>]><root>&own;</root>");
  agiru::dotnet::XmlReaderSettings settings;
  settings.DtdProcessing(agiru::dotnet::DtdProcessing::Prohibit());
  bool refused = false;
  std::string values;
  try {
    auto reader = agiru::dotnet::XmlReader::Create(input, settings);
    while (reader.Read()) { values += std::string(reader.Value().Value()); }
  } catch (const agiru::Error &) { refused = true; }
  const bool leaked = values.find("review-owned-entity-marker") != std::string::npos;
  std::printf("XML DTD Prohibit refuses: %s; reads own external file: %s\n",
              refused ? "yes" : "no", leaked ? "yes" : "no");
  return !refused || leaked;
}

int SequenceQuote() {
  agiru::Session session(kDsn);
  agiru::detail::Scope transaction;
  const std::string name = "review-20260928-quote'fixture";
  agiru::NumberSequence::Insert(name, 1, 1, false);
  try {
    const auto value = agiru::NumberSequence::Next(name, false);
    std::printf("quoted sequence name value: %lld; expected 1\n", static_cast<long long>(value));
    return value != 1;
  } catch (const agiru::Error &error) {
    std::printf("quoted sequence name refused: %s\n", error.what());
    return 1;
  }
}

int SequenceRanges() {
  const std::string name = "review-20260928-range-probe";
  agiru::Session owner(kDsn);
  if (agiru::NumberSequence::Exists(name, false)) {
    throw agiru::Error("fixture already exists; refusing to replace it");
  }
  agiru::NumberSequence::Insert(name, 1, 1, false);
  constexpr int kCount = 100;
  std::array<long long, 2> starts{};
  std::array<std::exception_ptr, 2> errors{};
  std::barrier together(2);
  const auto run = [&](std::size_t index) {
    bool crossed = false;
    try {
      agiru::Session worker(kDsn);
      together.arrive_and_wait();
      crossed = true;
      starts[index] = agiru::NumberSequence::Range(name, kCount, false);
    } catch (...) {
      errors[index] = std::current_exception();
      if (!crossed) { together.arrive_and_drop(); }
    }
  };
  std::thread first(run, 0);
  std::thread second(run, 1);
  first.join();
  second.join();
  agiru::NumberSequence::Delete(name, false);
  for (const auto &error : errors) { if (error) { std::rethrow_exception(error); } }
  const bool overlap = std::max(starts[0], starts[1]) <= std::min(starts[0], starts[1]) + kCount - 1;
  std::printf("concurrent sequence ranges: [%lld,%lld], [%lld,%lld]; overlap=%s\n",
              starts[0], starts[0] + kCount - 1, starts[1], starts[1] + kCount - 1, overlap ? "yes" : "no");
  return overlap;
}
}

int main() {
  try {
    const int failed = XmlPolicy() + SequenceQuote() + SequenceRanges();
    std::printf("boundary contract failures: %d\n", failed);
    return failed ? 1 : 0;
  } catch (const agiru::Error &error) {
    std::fprintf(stderr, "probe setup failure: %s\n", error.what());
    return 2;
  }
}
