#pragma once

#include <cstdint>

/// \file
/// \brief Session-owned data authorization shared by typed and reflected AL records.

namespace agiru {

struct TableDef;

/// \brief Independent AL TableData access operations; write requires Insert, Modify and Delete.
enum class TableOperation : std::uint8_t { Read, Insert, Modify, Delete };

/// \brief Trusted permission provider, not a grant inferred from a page or record identity.
/// Implementations must resolve the active session's exact user/company and effective rights.
/// Indirect rights require an authorized AL object execution context. This initial whole-table
/// boundary carries no row predicate: providers must refuse security-filtered policies until
/// row enforcement exists, never convert filtered access into unrestricted access. Shared
/// providers must be thread-safe and must not retain mutable user/worker state.
class TablePermissionAuthority {
public:
  virtual ~TablePermissionAuthority() = default;
  /// \param table Original immutable AL declaration, not a client-supplied selector.
  /// \param operation The actual TableData operation about to execute.
  /// \return Whether the current session may execute it; never grants execution on other objects.
  /// \throws Error for unavailable/unsupported policy, distinct from a denied permission.
  [[nodiscard]] virtual bool Allows(const TableDef &table, TableOperation operation) const = 0;
};

/// \brief Tests the active session's permission; no SQL/AL effect and no synthetic success.
/// \note Unaccounted SYSTEM/no-session native harnesses retain their explicit unrestricted
/// compatibility policy. Authenticated sessions without a provider refuse; they are not SUPER.
/// \param table Exact table declaration. \param operation The access being tested.
/// \return The provider's answer. \throws Error for unavailable/unsupported authority.
[[nodiscard]] bool HasTablePermission(const TableDef &table, TableOperation operation);

/// \brief Refuses a denied TableData operation before its physical or virtual data access.
/// \param table Exact declaration. \param operation Actual access kind.
/// \throws Error with Permission code and original table/operation identity on denial.
void RequireTablePermission(const TableDef &table, TableOperation operation);

namespace detail {
/// \brief Whole-table check for a live record; actual temporary buffers require no SQL rights.
/// \param record Actual typed/reflected buffer. \param table Exact declaration.
/// \param operation Requested physical/virtual access, not a trigger selector.
void RequireRecordPermission(const void *record, const TableDef &table, TableOperation operation);
}

/// \brief AL WritePermission: all three Insert, Modify and Delete permissions are required.
/// \param table Exact declaration. \return True only when all three operations are allowed.
[[nodiscard]] bool HasTableWritePermission(const TableDef &table);

}
