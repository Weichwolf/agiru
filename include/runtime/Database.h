#pragma once

#include "runtime/Error.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

/// \file
/// \brief The PostgreSQL connection, as far as SQL and nothing above it.

namespace agiru {

/// \brief An error raised by the database layer.
class DatabaseError : public Error {
public:
  using Error::Error;
};

/// \brief The rows a statement returned.
///
/// Values are text. The layer above turns them back into AL values, because this tier knows no AL
/// type -- `src/db/reaches` names nothing, and that is what keeps SQL out of the runtime and AL out
/// of SQL.
class Result {
public:
  /// \brief Takes ownership of a libpq result.
  /// \param handle The `PGresult`, already checked.
  explicit Result(void *handle);
  ~Result();

  Result(const Result &) = delete;
  Result &operator=(const Result &) = delete;

  /// \brief Moves ownership.
  /// \param o The result to move from.
  Result(Result &&o) noexcept;

  /// \brief Moves ownership.
  /// \param o The result to move from.
  /// \return This object.
  Result &operator=(Result &&o) noexcept;

  /// \return How many rows came back.
  [[nodiscard]] std::size_t Rows() const;

  /// \return How many columns came back.
  [[nodiscard]] std::size_t Columns() const;

  /// \brief Reads one value.
  /// \param row    Zero-based row.
  /// \param column Zero-based column.
  /// \return The value as text, or nothing when the column is null.
  [[nodiscard]] std::optional<std::string_view> Value(std::size_t row, std::size_t column) const;

private:
  void *handle_;
};

/// \brief A connection to PostgreSQL.
///
/// One connection is one session's private state. It is not shared and not copied, which is why the
/// type is move-only.
class Connection {
public:
  /// \brief Opens a connection.
  /// \param conninfo A libpq connection string or URI.
  /// \throws DatabaseError when the connection cannot be established.
  explicit Connection(const std::string &conninfo);
  ~Connection();

  Connection(const Connection &) = delete;
  Connection &operator=(const Connection &) = delete;

  /// \brief Moves ownership.
  /// \param o The connection to move from.
  Connection(Connection &&o) noexcept;

  /// \brief Moves ownership.
  /// \param o The connection to move from.
  /// \return This object.
  Connection &operator=(Connection &&o) noexcept;

  /// \brief Runs a statement with bound parameters.
  ///
  /// \param sql    The statement, with `$1`, `$2` and so on for parameters.
  /// \param params The parameter values as text; an empty optional binds SQL null.
  /// \return The rows, empty for a statement that returns none.
  /// \throws DatabaseError carrying the server's own message when the statement fails.
  ///
  /// \note Parameters are always BOUND and never pasted into the statement. A record's field values
  ///       come from data this runtime did not write.
  [[nodiscard]] Result Execute(std::string_view sql,
                               std::span<const std::optional<std::string>> params = {}) const;

  /// \brief Runs a statement that returns no rows.
  ///
  /// \param sql    The statement, with `$1`, `$2` and so on for parameters.
  /// \param params The parameter values as text; an empty optional binds SQL null.
  /// \throws DatabaseError carrying the server's own message when the statement fails.
  ///
  /// The same work as Execute(), named for the case where discarding the result is the intent
  /// rather than an oversight.
  void Run(std::string_view sql, std::span<const std::optional<std::string>> params = {}) const;

  /// \return True when a transaction block is open on this connection.
  ///
  /// A SAVEPOINT NEEDS ONE AND libpq DOES NOT OPEN IT. Outside a transaction block every statement
  /// commits on its own, `SAVEPOINT` is an error, and a boundary that rolled back nothing would
  /// look exactly like one that worked.
  [[nodiscard]] bool InTransaction() const;

private:
  void *handle_;
};

} // namespace agiru
