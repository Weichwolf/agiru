#pragma once

/// \file
/// \brief Database-owned rowversion allocation and active-transaction watermark.

namespace agiru {

class Connection;

/// \brief Installs and validates the versioned PostgreSQL rowversion primitive.
/// \param connection The database being explicitly provisioned, never an implicit template.
/// \throws DatabaseError for incompatible storage; existing counters are never reset.
/// \note The SQL functions next_rowversion_v1, last_rowversion_v1 and minimum_rowversion_v1
/// live in agiru_platform. Allocations are positive signed 64-bit values, database-wide and
/// nontransactional; gaps are allowed. No active allocator means a minimum of last + 1.
/// Exhaustion refuses rather than wrapping. This primitive alone does not activate AL methods
/// or install rowversion columns on ERP tables.
/// \note The platform reserves negative single-bigint advisory keys for transaction fences,
/// the two-integer key (ASCII AGRV, 0) for provisioning and (ASCII AGRV, 1) for publication.
/// The transaction-local agiru.rowversion_fence_v1 setting caches the owning transaction ID;
/// it is internal connection state, not user-controlled configuration. Each live transaction
/// retains its earliest fence, not a lock per row. Savepoint rollback restores both the fence
/// and its setting; Commit, rollback and disconnect release transaction locks automatically.
/// \note Provisioning uses an isolated savepoint in a caller transaction, or its own transaction
/// otherwise. Schema changes roll back together; allocations on existing storage do not.
void ProvisionRowVersions(const Connection &connection);

}
