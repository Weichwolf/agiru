#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"

#include <span>

/// \file
/// \brief What lets the runtime reach an AL table by NUMBER without knowing one by name.

namespace agiru {

/// \brief The generated declaration of one table. \see runtime/Table.h
template <typename T> struct TableTraits;

/// \brief One installed table: its declaration, and how to make and unmake a record of it.
///
/// \note THE TWO FUNCTION POINTERS ARE WHY THE RUNTIME NEED NOT KNOW THE TYPE. `RecordRef.Open(18)`
///       has to produce a record whose LAYOUT is table 18's, and only the generated class knows it.
///       This is the same shape `Instance<T>` uses for the same reason (board:0037).
struct TableEntry {
  const TableDef *table; ///< The declaration, which is `constexpr` data in `.rodata`.
  void *(*make)();       ///< Makes an empty record of that table.
  void (*free)(void *);  ///< Unmakes one.
};

/// \brief Makes an empty record.
/// \tparam T The generated table class.
/// \return The record, which the caller owns.
template <typename T> void *MakeRecord() {
  return new T();
}

/// \brief Unmakes a record `MakeRecord` made.
/// \tparam T The generated table class.
/// \param record The record.
template <typename T> void FreeRecord(void *record) {
  delete static_cast<T *>(record);
}

/// \brief The catalogue entry for one generated table.
///
/// \tparam T The generated table class.
///
/// \note IT IS `constexpr` AND IT LIVES IN `.rodata`. What is dynamic is the REGISTRATION -- one
///       pointer per table -- and not the data behind it, which is the line CLAUDE.md draws: object
///       metadata is emitted by the transpiler and never assembled at startup.
template <typename T>
inline constexpr TableEntry kTableEntry{
    .table = &TableTraits<T>::kTable, .make = &MakeRecord<T>, .free = &FreeRecord<T>};

/// \brief Adds one table to the catalogue.
/// \param entry The entry, which must outlive the process.
void RegisterTableEntry(const TableEntry *entry);

/// \brief Puts a generated table in the catalogue by existing.
///
/// \tparam T The generated table class.
///
/// \note A TABLE REGISTERS ITSELF FROM ITS OWN SOURCE, for the reason the test catalogue gives: one
///       catalogue file per app would have to include all 1 548 table headers in one translation
///       unit. What that costs is the ORDER, which the linker does not fix, so the lookup sorts.
template <typename T> struct RegisterTable {
  RegisterTable() { RegisterTableEntry(&kTableEntry<T>); }

  RegisterTable(const RegisterTable &) = delete;
  RegisterTable(RegisterTable &&) = delete;
  RegisterTable &operator=(const RegisterTable &) = delete;
  RegisterTable &operator=(RegisterTable &&) = delete;
  ~RegisterTable() = default;
};

/// \brief Finds an installed table by its AL number.
///
/// \param id The table number.
/// \return The entry, or `nullptr` when this binary carries no such table.
[[nodiscard]] const TableEntry *FindTable(TableId id);

/// \brief Every installed table, by number.
/// \return The entries, sorted by table number.
[[nodiscard]] std::span<const TableEntry *const> InstalledTables();

}
