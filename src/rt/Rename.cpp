#include "RenameCascade.h"

#include "Filter.h"
#include "RelationBranches.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"

#include <cctype>
#include <cstddef>
#include <map>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::detail {

namespace {

struct Reference {
  const TableEntry *entry;
  const FieldDef *field;
  RelationBranch branch;
};

std::string LowerKey(std::string_view text) {
  std::string key(text);
  for (char &c : key) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
  return key;
}

using ReferenceIndex = std::map<std::string, std::vector<Reference>>;

const ReferenceIndex &References() {
  static ReferenceIndex index;
  static std::once_flag built;
  std::call_once(built, [] {
    for (const TableEntry *entry : InstalledTables()) {
      for (const FieldDef &field : entry->table->fields) {
        if (field.relationTable.empty() && field.relation.empty()) { continue; }
        for (RelationBranch &branch : RelationBranches(field)) {
          if (branch.table.empty()) { continue; }
          const std::string key = LowerKey(branch.table);
          index[key].push_back(
              Reference{.entry = entry, .field = &field, .branch = std::move(branch)});
        }
      }
    }
  });
  return index;
}

struct Made {
  const TableEntry *entry;
  void *record;
  Made(const TableEntry *e) : entry(e), record(e->make()) {}
  Made(const Made &) = delete;
  Made &operator=(const Made &) = delete;
  ~Made() { entry->free(record); }
};

const FieldDef *FieldNamed(const TableDef &table, std::string_view name) {
  for (const FieldDef &field : table.fields) {
    if (SameName(field.name, name)) { return &field; }
  }
  return nullptr;
}

bool ConstantHolds(const RelationTerm &term, const void *before, const TableDef &table) {
  const FieldDef *target = FieldNamed(table, term.field);
  if (target == nullptr) { return false; }
  const std::string text =
      SameName(term.kind, "filter") ? term.inner : Literally(term.inner);
  return Matches(ParseFilter(text), FieldText(before, *target), *target);
}

bool Narrowed(RecordState &state,
              const TableDef &referring,
              const Reference &reference,
              const void *before,
              const TableDef &table) {
  for (const RelationTerm &term : reference.branch.conditions) {
    const FieldDef *own = FieldNamed(referring, term.field);
    if (own == nullptr) { return false; }
    if (term.kind.empty() || SameName(term.kind, "const")) {
      Narrow(state, own->no, Literally(term.inner));
    } else if (SameName(term.kind, "filter")) {
      Narrow(state, own->no, term.inner);
    } else {
      return false;
    }
  }
  for (const RelationTerm &term : reference.branch.filters) {
    if (SameName(term.kind, "field")) {
      const FieldDef *own = FieldNamed(referring, term.inner);
      const FieldDef *target = FieldNamed(table, term.field);
      if (own == nullptr || target == nullptr) { return false; }
      Narrow(state, own->no, Literally(FieldText(before, *target)));
    } else if (!ConstantHolds(term, before, table)) {
      return false;
    }
  }
  return true;
}

void Rewrite(const Reference &reference,
             const void *before,
             const TableDef &table,
             const std::string &oldText,
             const std::string &newText) {
  const TableDef &referring = *reference.entry->table;
  const FieldDef &field = *reference.field;
  const Made row(reference.entry);
  RecordState &state = reinterpret_cast<StateHandle *>(row.record)->Ensure();
  Narrow(state, field.no, Literally(oldText));
  if (!Narrowed(state, referring, reference, before, table)) { return; }
  if (!RuntimeFindSet(row.record, referring)) { return; }
  const bool inKey = !referring.keys.empty() &&
                     std::ranges::find(referring.keys[0].fields, field.no) !=
                         referring.keys[0].fields.end();
  do {
    if (inKey) {
      const Made was(reference.entry);
      reference.entry->copy(was.record, row.record);
      SetFieldText(row.record, field, newText);
      static_cast<void>(RuntimeRename(row.record, was.record, referring));
    } else {
      SetFieldText(row.record, field, newText);
      static_cast<void>(RuntimeModify(row.record, referring));
    }
  } while (RuntimeNext(row.record, referring, 1) != 0);
}

}

void CascadeRename(const void *record, const void *before, const TableDef &table) {
  if (table.keys.empty() || RuntimeIsTemporary(record)) { return; }
  const ReferenceIndex &index = References();
  const auto found = index.find(LowerKey(table.name));
  if (found == index.end()) { return; }
  const std::span<const FieldNo> key = table.keys[0].fields;
  for (std::size_t position = 0; position < key.size(); ++position) {
    const FieldDef *def = nullptr;
    for (const FieldDef &candidate : table.fields) {
      if (candidate.no == key[position]) { def = &candidate; }
    }
    if (def == nullptr) { continue; }
    const std::string oldText = FieldText(before, *def);
    const std::string newText = FieldText(record, *def);
    if (oldText == newText) { continue; }
    for (const Reference &reference : found->second) {
      const bool targetsThisKey = reference.branch.field.empty()
                                      ? position == 0
                                      : SameName(reference.branch.field, def->name);
      if (!targetsThisKey) { continue; }
      Rewrite(reference, before, table, oldText, newText);
    }
  }
}

}
