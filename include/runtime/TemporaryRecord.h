#pragma once

/// \file
/// \brief Opaque temporary-row checks and borrowing without typed table implementations.

namespace agiru::detail {

/// \brief Tests the actual temporary-row state of a record buffer.
/// \param record The live typed or reflected record buffer.
/// \return Whether the buffer carries temporary rows.
[[nodiscard]] bool RuntimeIsTemporary(const void *record);

/// \brief Borrows another record's temporary rows while preserving the borrower's filters.
/// \param record The borrowing record buffer.
/// \param from The record buffer owning the temporary rows.
void RuntimeBorrowTemporary(void *record, const void *from);

}
