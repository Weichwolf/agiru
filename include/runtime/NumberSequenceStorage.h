#pragma once

/// \file
/// \brief Provisioning the database-owned AL number-sequence primitive.

namespace agiru {

class Connection;

/// \brief Installs the versioned registry and PostgreSQL sequence operations.
/// \param connection The database being explicitly provisioned, never an implicit template.
/// \throws DatabaseError for incompatible storage or unmigrated legacy sequences.
/// \note Sequence identities are registry IDs, not compiler-dependent hashes. Schema changes
/// participate in the caller's transaction; consumption of existing sequences does not roll back.
void ProvisionNumberSequences(const Connection &connection);

}
