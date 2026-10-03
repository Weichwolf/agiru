#include "type/NumberSequence.h"

#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace agiru {

namespace {

struct Request {
  std::string_view operation;
  std::string_view name;
  Boolean companySpecific;
  BigInteger seed = 0;
  BigInteger increment = 1;
  Integer count = 1;
};

Result Operate(const Request &request) {
  const Session &session = Session::Current();
  if (request.name.find('\0') != std::string_view::npos ||
      session.CompanyName().find('\0') != std::string_view::npos) {
    throw Error("NumberSequence: identities cannot contain a null character");
  }
  constexpr std::size_t kParameterCount = 7;
  const std::array<std::optional<std::string>, kParameterCount> parameters{
      std::string(request.operation),
      std::string(request.name),
      request.companySpecific ? "true" : "false",
      std::string(session.CompanyName()),
      std::to_string(request.seed),
      std::to_string(request.increment),
      std::to_string(request.count)};
  Result result = session.Database().Execute(
      "SELECT value, increment, present FROM agiru_platform.number_sequence_v1("
      "$1::text, $2::text, $3::boolean, $4::text, $5::bigint, $6::bigint, $7::integer)",
      parameters);
  if (result.Rows() == 0) {
    throw Error("the number sequence " + std::string(request.name) + " does not exist");
  }
  return result;
}

BigInteger RequiredValue(const Result &row, std::size_t column) {
  const std::optional<std::string_view> value = row.Value(0, column);
  if (!value) { throw DatabaseError("NumberSequence: storage returned an unexpected null"); }
  return std::stoll(std::string(*value));
}

}

BigInteger NumberSequence::Current(std::string_view Name, Boolean CompanySpecific) {
  return RequiredValue(
      Operate({.operation = "current", .name = Name, .companySpecific = CompanySpecific}), 0);
}

void NumberSequence::Delete(std::string_view Name, Boolean CompanySpecific) {
  const Result discarded =
      Operate({.operation = "delete", .name = Name, .companySpecific = CompanySpecific});
}

Boolean NumberSequence::Exists(std::string_view Name, Boolean CompanySpecific) {
  return Operate({.operation = "exists", .name = Name, .companySpecific = CompanySpecific})
             .Value(0, 2) == "t";
}

void NumberSequence::Insert(std::string_view Name,
                            BigInteger Seed,
                            BigInteger Increment,
                            Boolean CompanySpecific) {
  const Result discarded = Operate({.operation = "insert",
                                    .name = Name,
                                    .companySpecific = CompanySpecific,
                                    .seed = Seed,
                                    .increment = Increment});
}

BigInteger NumberSequence::Next(std::string_view Name, Boolean CompanySpecific) {
  return Range(Name, 1, CompanySpecific);
}

BigInteger NumberSequence::Range(std::string_view Name, Integer Count, BigInteger &Increment) {
  return Range(Name, Count, Increment, true);
}

BigInteger NumberSequence::Range(std::string_view Name,
                                 Integer Count,
                                 BigInteger &Increment,
                                 Boolean CompanySpecific) {
  const Result reserved = Operate(
      {.operation = "reserve", .name = Name, .companySpecific = CompanySpecific, .count = Count});
  const BigInteger first = RequiredValue(reserved, 0);
  Increment = RequiredValue(reserved, 1);
  return first;
}

BigInteger NumberSequence::Range(std::string_view Name, Integer Count, Boolean CompanySpecific) {
  BigInteger increment = 0;
  return Range(Name, Count, increment, CompanySpecific);
}

void NumberSequence::Restart(std::string_view Name, BigInteger Seed, Boolean CompanySpecific) {
  const Result discarded = Operate(
      {.operation = "restart", .name = Name, .companySpecific = CompanySpecific, .seed = Seed});
}

}
