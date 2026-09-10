#pragma once

#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "meta/ProfileDef.h"
#include "meta/TableDef.h"
#include "type/Action.h"

#include <span>
#include <string_view>

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
  /// \brief `Record.Validate(Field, Text)` by number, for a `FieldRef` that holds no type.
  void (*validate)(void *record, FieldNo no, std::string_view text);
  /// \brief Copies one record of this table into another, fields and filters, for a `RecordRef`
  ///        that takes a record it does not know the type of and must OWN it.
  void (*copy)(void *to, const void *from);
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
    .table = &TableTraits<T>::kTable,
    .make = &MakeRecord<T>,
    .free = &FreeRecord<T>,
    .validate = [](void *record,
                   FieldNo no,
                   std::string_view text) { static_cast<T *>(record)->ValidateText(no, text); },
    .copy = [](void *to,
               const void *from) { static_cast<T *>(to)->Copy(*static_cast<const T *>(from)); }};

/// \brief Adds one table to the catalogue.
/// \param entry The entry, which must outlive the process.
void RegisterTableEntry(const TableEntry *entry);

/// \brief What the runtime knows about a generated page: its declaration and how to run it.
struct PageEntry {
  const PageDef *page; ///< The declaration, `constexpr` data in `.rodata`.
  /// \brief Runs the page headless, the way `Page.Run`/`Page.RunModal` do on the generated class.
  /// \param modal    Whether it is `RunModal`.
  /// \param record   The record passed, or `nullptr`.
  /// \param table    Its declaration, or `nullptr`.
  /// \param writable Whether the caller's record takes the page's record back when it closes --
  ///                 true for a `var` record, false for a const one.
  /// \return The action the page closed with.
  ::agiru::Action (*run)(bool modal, void *record, const TableDef *table, bool writable);
};

/// \brief Puts a page in the catalogue, once per generated page, at load time.
/// \param entry The entry, which lives for the program.
void RegisterPageEntry(const PageEntry *entry);

/// \brief Finds a page by its number.
/// \param id The number.
/// \return The entry, or `nullptr` when this build carries no such page.
[[nodiscard]] const PageEntry *FindPage(PageId id);

/// \brief The page a lookup on a table opens: its `LookupPageId`, else the first `List` page whose
///        `SourceTable` it is, else any page on it (`devenv-lookuppageid-property.md`).
/// \param table The related table.
/// \return The entry, or `nullptr` when this build carries no page on the table.
[[nodiscard]] const PageEntry *FindLookupPage(const TableDef &table);

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

/// \brief The installed table of an AL NAME, which is how a `TableRelation` names its target.
/// \param name The AL table name, compared without regard to case.
/// \return The entry, or nothing when this binary carries no such table.
[[nodiscard]] const TableEntry *FindTable(std::string_view name);

/// \brief Every installed table, by number.
/// \return The entries, sorted by table number.
[[nodiscard]] std::span<const TableEntry *const> InstalledTables();

/// \brief Registers a translated `profile` object; the generated source does this once, at load.
/// \param profile The declaration, `constexpr` data in `.rodata`.
void RegisterProfileEntry(const ProfileDef *profile);

/// \brief The static registration a generated profile source makes.
struct RegisterProfile {
  /// \brief Registers the profile.
  /// \param profile The declaration.
  explicit RegisterProfile(const ProfileDef *profile) { RegisterProfileEntry(profile); }

  RegisterProfile(const RegisterProfile &) = delete;
  RegisterProfile(RegisterProfile &&) = delete;
  RegisterProfile &operator=(const RegisterProfile &) = delete;
  RegisterProfile &operator=(RegisterProfile &&) = delete;
  ~RegisterProfile() = default;
};

/// \brief Every installed profile, in the order the sources registered them.
/// \return The declarations.
[[nodiscard]] std::span<const ProfileDef *const> InstalledProfiles();

struct CodeunitEntry;

/// \brief Every installed codeunit, in the order the sources registered them.
/// \return The entries.
[[nodiscard]] std::span<const CodeunitEntry *const> InstalledCodeunits();

/// \brief Every installed page, in the order the sources registered them.
/// \return The entries.
[[nodiscard]] std::span<const PageEntry *const> InstalledPages();

}
