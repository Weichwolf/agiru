#include "runtime/Database.h"

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <libpq-fe.h>

namespace agiru {

namespace {

PGresult *Handle(void *h) {
  return static_cast<PGresult *>(h);
}

PGconn *Conn(void *h) {
  return static_cast<PGconn *>(h);
}

} // namespace

Result::Result(void *handle) : handle_(handle) {}

Result::~Result() {
  if (handle_ != nullptr) { PQclear(Handle(handle_)); }
}

Result::Result(Result &&o) noexcept : handle_(std::exchange(o.handle_, nullptr)) {}

Result &Result::operator=(Result &&o) noexcept {
  if (this != &o) {
    if (handle_ != nullptr) { PQclear(Handle(handle_)); }
    handle_ = std::exchange(o.handle_, nullptr);
  }
  return *this;
}

std::size_t Result::Rows() const {
  return handle_ == nullptr ? 0 : static_cast<std::size_t>(PQntuples(Handle(handle_)));
}

std::size_t Result::Columns() const {
  return handle_ == nullptr ? 0 : static_cast<std::size_t>(PQnfields(Handle(handle_)));
}

std::optional<std::string_view> Result::Value(std::size_t row, std::size_t column) const {
  if (handle_ == nullptr) { throw DatabaseError("Result: no rows"); }
  const auto r = static_cast<int>(row);
  const auto c = static_cast<int>(column);
  if (row >= Rows() || column >= Columns()) { throw DatabaseError("Result: out of range"); }
  if (PQgetisnull(Handle(handle_), r, c) == 1) { return std::nullopt; }
  return std::string_view(PQgetvalue(Handle(handle_), r, c),
                          static_cast<std::size_t>(PQgetlength(Handle(handle_), r, c)));
}

Connection::Connection(const std::string &conninfo) : handle_(PQconnectdb(conninfo.c_str())) {
  if (PQstatus(Conn(handle_)) != CONNECTION_OK) {
    const std::string message = PQerrorMessage(Conn(handle_));
    PQfinish(Conn(handle_));
    handle_ = nullptr;
    throw DatabaseError("Connection: " + message);
  }
}

Connection::~Connection() {
  if (handle_ != nullptr) { PQfinish(Conn(handle_)); }
}

Connection::Connection(Connection &&o) noexcept : handle_(std::exchange(o.handle_, nullptr)) {}

Connection &Connection::operator=(Connection &&o) noexcept {
  if (this != &o) {
    if (handle_ != nullptr) { PQfinish(Conn(handle_)); }
    handle_ = std::exchange(o.handle_, nullptr);
  }
  return *this;
}

Result Connection::Execute(std::string_view sql,
                           std::span<const std::optional<std::string>> params) const {
  std::vector<const char *> values;
  values.reserve(params.size());
  for (const std::optional<std::string> &p : params) {
    values.push_back(p.has_value() ? p->c_str() : nullptr);
  }

  PGresult *result = PQexecParams(Conn(handle_),
                                  std::string(sql).c_str(),
                                  static_cast<int>(values.size()),
                                  nullptr,
                                  values.data(),
                                  nullptr,
                                  nullptr,
                                  0);

  const ExecStatusType status = PQresultStatus(result);
  if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
    const std::string message = PQresultErrorMessage(result);
    PQclear(result);
    throw DatabaseError(message + "statement: " + std::string(sql));
  }
  return Result(result);
}

void Connection::Run(std::string_view sql,
                     std::span<const std::optional<std::string>> params) const {
  const Result discarded = Execute(sql, params);
}

} // namespace agiru
