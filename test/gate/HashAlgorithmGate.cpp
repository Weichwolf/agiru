#include "dotnet/HashAlgorithm.h"
#include "dotnet/Regex.h"
#include "runtime/ErrorValue.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/Variant.h"

#include "Check.h"
#include "Reference.h"

#include <array>
#include <cstddef>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>

namespace {

using agiru::dotnet::Array;
using agiru::dotnet::HashAlgorithm;

struct Vector {
  std::string_view name;
  std::string_view abc;
  std::string_view empty;
};

constexpr std::array<Vector, 5> kVectors{{
    {.name = "MD5",
     .abc = "900150983CD24FB0D6963F7D28E17F72",
     .empty = "D41D8CD98F00B204E9800998ECF8427E"},
    {.name = "SHA1",
     .abc = "A9993E364706816ABA3E25717850C26C9CD0D89D",
     .empty = "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709"},
    {.name = "SHA256",
     .abc = "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD",
     .empty = "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855"},
    {.name = "SHA384",
     .abc = "CB00753F45A35E8BB5A03D699AC65007272C32AB0EDED1631A8B605A43FF5BED8086072BA1E7CC"
            "2358BAECA134C825A7",
     .empty =
         "38B060A751AC96384CD9327EB1B1E36A21FDB71114BE07434C0CC7BF63F6E1DA274EDEBFE76F65FBD51AD"
         "2F14898B95B"},
    {.name = "SHA512",
     .abc = "DDAF35A193617ABACC417349AE20413112E6FA4E89A97EA20A9EEEE64B55D39A2192992A274FC"
            "1A836BA3C23A3FEEBBD454D4423643CE80E2A9AC94FA54CA49F",
     .empty =
         "CF83E1357EEFB8BDF1542850D66D8007D620E4050B5715DC83F4A921D36CE9CE47D0D13C5D85F2B0FF831"
         "8D2877EEC2F63B931BD47417A81A538327AF927DA3E"},
}};

Array Bytes(std::string_view value) {
  Array result;
  for (const char cell : value) {
    result.Add(agiru::Variant{agiru::Integer{static_cast<unsigned char>(cell)}});
  }
  return result;
}

std::string Raw(const Array &bytes) {
  std::string result;
  for (const auto &cell : bytes) {
    const agiru::Integer value = cell;
    CHECK_TRUE("digest cells are unsigned bytes", cell.IsInteger() && value >= 0 && value <= 255);
    result.push_back(static_cast<char>(value));
  }
  return result;
}

template <typename Operation> bool Fails(Operation operation) {
  try {
    operation();
  } catch (const agiru::Error &) { return true; }
  return false;
}

void ValuesAndLifetime() {
  const auto abc = Bytes("abc");
  for (const auto &vector : kVectors) {
    auto hash = HashAlgorithm::Create(vector.name);
    CHECK_TRUE("named factory binds a fresh reference", !hash.IsNullObject());
    auto digest = hash.ComputeHash(abc);
    CHECK_TEXT("each algorithm has its own abc digest", Raw(digest), gate::Unhex(vector.abc));
    CHECK_TEXT("empty array hashes empty bytes",
               Raw(hash.ComputeHash(Bytes(""))),
               gate::Unhex(vector.empty));
    CHECK_TEXT("slice preserves exact byte boundaries",
               Raw(hash.ComputeHash(Bytes("xabcx"), 1, 3)),
               gate::Unhex(vector.abc));
    CHECK_TEXT("end-position empty slice is valid",
               Raw(hash.ComputeHash(abc, 3, 0)),
               gate::Unhex(vector.empty));
    digest.SetValue(agiru::Variant{agiru::Integer{0}}, 0);
    CHECK_TEXT("a returned array cannot change later hashes",
               Raw(hash.ComputeHash(abc)),
               gate::Unhex(vector.abc));
    auto alias = hash;
    alias.Dispose();
    alias.Dispose();
    CHECK_TRUE("disposed reference remains bound", !hash.IsNullObject());
    CHECK_TRUE("Dispose invalidates every alias", Fails([&] { (void)hash.ComputeHash(abc); }));
    hash = HashAlgorithm::Create(vector.name);
    CHECK_TEXT("rebinding one wrapper creates a fresh digest",
               Raw(hash.ComputeHash(abc)),
               gate::Unhex(vector.abc));
    CHECK_TRUE("rebinding does not resurrect disposed aliases",
               Fails([&] { (void)alias.ComputeHash(abc); }));
  }
}

void Refusals() {
  HashAlgorithm unbound;
  CHECK_TRUE("default declaration is null", unbound.IsNullObject());
  CHECK_TRUE("null instance cannot hash", Fails([&] { (void)unbound.ComputeHash(Bytes("")); }));
  CHECK_TRUE("null instance cannot Dispose", Fails([&] { unbound.Dispose(); }));
  CHECK_TRUE("parameterless factory refuses", Fails([] { (void)HashAlgorithm::Create(); }));
  CHECK_TRUE("known keyed algorithms refuse rather than masquerading as unknown",
             Fails([] { (void)HashAlgorithm::Create("HMACSHA256"); }));
  for (const auto *const name : {"", "unknown", "SHA-1", "SHA256Managed", " SHA256", "SHA256 "}) {
    CHECK_TRUE("unknown names retain null result", HashAlgorithm::Create(name).IsNullObject());
  }
  auto hash = HashAlgorithm::Create("SHA256");
  const auto bytes = Bytes("abc");
  constexpr std::array<std::array<agiru::Integer, 2>, 5> regions{
      {{-1, 0}, {0, -1}, {4, 0}, {1, 3}, {std::numeric_limits<agiru::Integer>::max(), 1}}};
  for (const auto &region : regions) {
    CHECK_TRUE("invalid byte regions refuse without overflow",
               Fails([&] { (void)hash.ComputeHash(bytes, region[0], region[1]); }));
  }
  for (const auto value : {-1, 256}) {
    Array invalid;
    invalid.Add(agiru::Variant{agiru::Integer{value}});
    CHECK_TRUE("out-of-range cells are not masked into bytes",
               Fails([&] { (void)hash.ComputeHash(invalid); }));
  }
  Array invalid;
  invalid.Add(agiru::Variant{agiru::Text<0>{"65"}});
  CHECK_TRUE("text cells are not implicitly parsed as bytes",
             Fails([&] { (void)hash.ComputeHash(invalid); }));
}

void LargeBlocks() {
  const std::string input(8193, 'a');
  const auto bytes = Bytes(input);
  auto hash = HashAlgorithm::Create("SHA256");
  CHECK_TEXT("multiple bounded blocks equal an independently qualified digest",
             Raw(hash.ComputeHash(bytes)),
             gate::Unhex("9C10C48D1F1D6618DB88FDE2C25409181C9201ED34EC6815D62BCF57C10D177B"));
}

void Reference(std::string_view path) {
  std::ifstream input{std::string(path)};
  if (!input) { throw agiru::Error("Hash reference cannot be opened"); }
  std::size_t rows = 0;
  for (std::string line; std::getline(input, line);) {
    ++rows;
    if (line.starts_with("bytes\t")) {
      const auto fields = gate::ReferenceFields<4>(line);
      auto hash = HashAlgorithm::Create(fields[1]);
      CHECK_TEXT("CLR byte digest agrees",
                 Raw(hash.ComputeHash(Bytes(gate::Unhex(fields[2])))),
                 gate::Unhex(fields[3]));
    } else if (line.starts_with("name\t")) {
      const auto fields = gate::ReferenceFields<3>(line);
      if (fields[1] == "HMACSHA256") {
        CHECK_TRUE("CLR keyed factory remains an explicit runtime gap",
                   Fails([&] { (void)HashAlgorithm::Create(fields[1]); }));
        continue;
      }
      auto hash = HashAlgorithm::Create(fields[1]);
      if (fields[2] == "null") {
        CHECK_TRUE("CLR factory null agrees", hash.IsNullObject());
      } else {
        CHECK_TEXT("CLR factory alias agrees",
                   Raw(hash.ComputeHash(Bytes("abc"))),
                   gate::Unhex(fields[2]));
      }
    } else if (line.starts_with("slice\t") || line.starts_with("slice-empty\t") ||
               line.starts_with("independent\t")) {
      const auto fields = gate::ReferenceFields<3>(line);
      auto hash = HashAlgorithm::Create(fields[1]);
      const auto digest = fields[0] == "slice-empty" ? hash.ComputeHash(Bytes("xabcx"), 5, 0)
                                                     : hash.ComputeHash(Bytes("xabcx"), 1, 3);
      CHECK_TEXT("CLR exact range agrees", Raw(digest), gate::Unhex(fields[2]));
    } else if (line.starts_with("range\t")) {
      const auto fields = gate::ReferenceFields<4>(line);
      auto hash = HashAlgorithm::Create("SHA256");
      CHECK_TRUE("CLR range errors agree", Fails([&] {
                   (void)hash.ComputeHash(Bytes("xabcx"),
                                          std::stoi(std::string(fields[1])),
                                          std::stoi(std::string(fields[2])));
                 }));
    } else if (line.starts_with("disposed\t")) {
      const auto fields = gate::ReferenceFields<3>(line);
      auto hash = HashAlgorithm::Create(fields[1]);
      auto alias = hash;
      alias.Dispose();
      CHECK_TRUE("CLR shared disposal agrees",
                 Fails([&] { (void)hash.ComputeHash(Bytes("abc")); }));
    } else if (line.starts_with("default\t")) {
      (void)gate::ReferenceFields<2>(line);
      CHECK_TRUE("CLR parameterless refusal agrees", Fails([] { (void)HashAlgorithm::Create(); }));
    } else {
      throw agiru::Error("Unknown hash reference row");
    }
  }
  CHECK_TRUE("complete CLR reference population retained", rows == 1424);
}

}

int main(int argc, char **argv) {
  return gate::Run("HashAlgorithm", [&] {
    ValuesAndLifetime();
    Refusals();
    LargeBlocks();
    if (argc == 2) { Reference(argv[1]); }
  });
}
