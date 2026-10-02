#pragma once

#include "meta/TableDef.h"
#include "runtime/Database.h"

#include <string>

/// \file
/// \brief Turning an AL table declaration into a schema.

namespace agiru {

/// \brief Creates the table the declaration describes.
///
/// \param connection The database.
/// \param table      The declaration.
/// \throws DatabaseError when the statement fails.
///
/// Identifiers keep their AL spelling and are quoted, so a column is `"Work Type Code"`. That is
/// BC's own convention, and matching it is what will let the CRONUS load map column for column
/// (board:0004).
void CreateTable(const Connection &connection, const TableDef &table);

/// \brief Drops the table if it exists.
/// \param connection The database.
/// \param table      The declaration.
/// \throws DatabaseError when the statement fails.
void DropTable(const Connection &connection, const TableDef &table);

/// \brief Creates every table this binary carries that the database does not have.
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
