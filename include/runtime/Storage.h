#pragma once

#include "meta/TableDef.h"
#include "runtime/Database.h"

#include <string>

/// \file
/// \brief Turning an AL table declaration into a schema.

namespace agiru {

/// \brief Reject an unavailable runtime table provider before any physical storage operation.
/// \param table The declaration whose provider is needed.
/// \throws Error with the original table identity and providerRefusal; never returns false.
void RequireTableProvider(const TableDef &table);

/// \brief Creates the table the declaration describes.
///
/// \param connection The database.
/// \param table      The declaration.
/// \throws DatabaseError when the statement fails.
///
/// Identifiers keep their AL spelling and are quoted, so a column is `"Work Type Code"`. That is
/// BC's own convention, and matching it is what will let the CRONUS load map column for column
/// (board:0004).
/// \note SqlTimestamp fields share one bigint timestamp column and its database-owned allocator;
///       their source names never create independent columns or accept supplied versions.
void CreateTable(const Connection &connection, const TableDef &table);

/// \brief Synchronizes one visible table with its immutable declaration.
/// \param connection The database being explicitly provisioned.
/// \param table The declaration; the connection's search path selects its schema.
/// \note Creates absent storage, adds missing columns and widens bounded Text/Code fields.
///       Preserves existing data and obsolete columns. Rowversion aliases share one allocator
///       column; incompatible existing timestamp storage requires an explicit migration.
/// \throws Error for incompatible declarations/storage; DatabaseError for failed SQL.
void ProvisionTable(const Connection &connection, const TableDef &table);

/// \brief Explicitly installs private own-write metadata on existing registered versioned tables.
/// \param connection The operator-selected database; no request invokes this migration.
/// \note Preserves business, audit and rowversion values; absent/provider-refused tables stay
/// absent.
///       One UUID column identifies own uncommitted writes without transaction-ID wrap assumptions.
///       Migration is atomic in its own transaction or a savepoint of the caller's transaction.
/// \throws Error for incompatible metadata, DatabaseError for schema failures.
void ProvisionWriteOwnership(const Connection &connection);

/// \brief Drops the table if it exists.
/// \param connection The database.
/// \param table      The declaration.
/// \throws DatabaseError when the statement fails.
void DropTable(const Connection &connection, const TableDef &table);

/// \brief Creates every table this binary carries that the database does not have.
/// \note Adds missing stored fields and widens existing bounded Text/Code columns to the
///       declared length without truncating values or narrowing wider columns. Other type
///       migrations are not implemented by this bootstrap operation.
/// \note Missing rowversion storage is populated once per existing row by PostgreSQL, without
///       overwriting ordinary or audit fields. Existing incompatible timestamp storage refuses;
///       provisioning never resets the database-wide counter.
///
/// \param into The database.
/// \throws DatabaseError when a statement fails.
///
/// \note IT IS THE TEST RUNNER'S OWN DATABASE, not the master. A test writes rows, and a table this
///       build knows and PostgreSQL does not is `relation "Sales Header" does not exist` -- an
///       error about provisioning that reads like a defect in the code under test. The declarations
///       are `constexpr` data in `.rodata`, so this is a walk over the catalogue and not a schema
///       kept anywhere.
void ProvisionInstalled(const Connection &into);

/// \brief The SQL column type an AL field type maps to.
///
/// \param def The field.
/// \return The PostgreSQL type; `numeric(38,20)` for a Decimal, because that is the width BC
///         stores.
[[nodiscard]] std::string ColumnType(const FieldDef &def);

}
