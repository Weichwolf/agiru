#include "runtime/Database.h"
#include "runtime/ProcessDiagnostics.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
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

void TraceStatement(std::string_view sql, std::span<const std::optional<std::string>> params) {
  if (!detail::TraceSql()) { return; }
  std::string line = "sql: " + std::string(sql);
  for (const std::optional<std::string> &param : params) {
    line += " | ";
    line += param.has_value() ? *param : std::string("NULL");
  }
  line += '\n';
  std::fputs(line.c_str(), stderr);
}

void TraceRows(PGresult *result) {
  if (!detail::TraceSqlRows() || PQresultStatus(result) != PGRES_TUPLES_OK) { return; }
  std::string line = "  -> " + std::to_string(PQntuples(result)) + " row(s)";
  if (PQntuples(result) > 0) {
    for (int column = 0; column < PQnfields(result); ++column) {
      line += column == 0 ? ": " : " | ";
      line += PQgetisnull(result, 0, column) != 0 ? "NULL" : PQgetvalue(result, 0, column);
    }
  }
  line += '\n';
  std::fputs(line.c_str(), stderr);
}

}

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

std::size_t Result::Affected() const {
  if (handle_ == nullptr) { return 0; }
  const char *count = PQcmdTuples(Handle(handle_));
  if (count == nullptr || *count == '\0') { return 0; }
  return static_cast<std::size_t>(std::strtoull(count, nullptr, 10));
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
  Close();
}

void Connection::Close() noexcept {
  if (handle_ != nullptr) { PQfinish(Conn(handle_)); }
  handle_ = nullptr;
}

bool Connection::IsOpen() const noexcept {
  return handle_ != nullptr && PQstatus(Conn(handle_)) == CONNECTION_OK;
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
  if (handle_ == nullptr) {
    throw DatabaseError("the connection is closed and this statement cannot run: " +
                        std::string(sql));
  }
  std::vector<const char *> values;
  values.reserve(params.size());
  for (const std::optional<std::string> &p : params) {
    values.push_back(p.has_value() ? p->c_str() : nullptr);
  }
  TraceStatement(sql, params);

  PGresult *result = PQexecParams(Conn(handle_),
                                  std::string(sql).c_str(),
                                  static_cast<int>(values.size()),
                                  nullptr,
                                  values.data(),
                                  nullptr,
                                  nullptr,
                                  0);
  Result owned(result);

  const ExecStatusType status = PQresultStatus(result);
  if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
    const std::string message = PQresultErrorMessage(result);
    throw DatabaseError(message + "statement: " + std::string(sql));
  }
  TraceRows(result);
  return owned;
}

void Connection::Run(std::string_view sql,
                     std::span<const std::optional<std::string>> params) const {
  const Result discarded = Execute(sql, params);
}

}

namespace agiru {

bool Connection::InTransaction() const {
  if (handle_ == nullptr) { throw DatabaseError("the connection is closed"); }
  const PGTransactionStatusType status = PQtransactionStatus(static_cast<PGconn *>(handle_));
  return status == PQTRANS_INTRANS || status == PQTRANS_INERROR;
}

bool Connection::InFailedTransaction() const {
  if (handle_ == nullptr) { throw DatabaseError("the connection is closed"); }
  return PQtransactionStatus(static_cast<PGconn *>(handle_)) == PQTRANS_INERROR;
}

}
