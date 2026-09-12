#include "runtime/Page.h"

#include "meta/PageDef.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Record.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Relation.h"

#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {

namespace {

std::string_view Trimmed(std::string_view text) {
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
    text.remove_prefix(1);
  }
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
    text.remove_suffix(1);
  }
  return text;
}

std::string_view Unquoted(std::string_view text) {
  text = Trimmed(text);
  if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
    return text.substr(1, text.size() - 2);
  }
  return text;
}

bool SameName(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) { return false; }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

std::vector<std::string_view> NamesIn(std::string_view list) {
  std::vector<std::string_view> names;
  bool quoted = false;
  std::size_t start = 0;
  for (std::size_t i = 0; i <= list.size(); ++i) {
    if (i < list.size() && list[i] == '"') { quoted = !quoted; }
    if (i == list.size() || (list[i] == ',' && !quoted)) {
      const std::string_view name = Unquoted(list.substr(start, i - start));
      if (!name.empty()) { names.push_back(name); }
      start = i + 1;
    }
  }
  return names;
}

const FieldDef *FieldNamed(const TableDef &table, std::string_view name) {
  for (const FieldDef &field : table.fields) {
    if (SameName(field.name, name)) { return &field; }
  }
  return nullptr;
}

std::span<const FieldNo> CaptionFieldsOf(const TableDef &table) {
  if (!table.dataCaptionFields.empty()) { return table.dataCaptionFields; }
  if (!table.keys.empty()) { return table.keys[0].fields; }
  return {};
}

void Append(std::string &caption, std::string_view value) {
  if (Trimmed(value).empty()) { return; }
  if (!caption.empty()) { caption += ' '; }
  caption += Trimmed(value);
}

std::string CaptionOfRecord(const void *record, const TableDef &table) {
  std::string caption;
  for (const FieldNo no : CaptionFieldsOf(table)) {
    const FieldDef *def = Field(table, no);
    if (def != nullptr) { Append(caption, FieldText(record, *def)); }
  }
  return caption;
}

std::string CaptionOfRelated(const TableDef &target, const FieldDef *column, std::string_view value) {
  const std::span<const FieldNo> key = target.keys.empty() ? std::span<const FieldNo>{} : target.keys[0].fields;
  const FieldDef *keyField = column != nullptr ? column : (key.size() == 1 ? Field(target, key[0]) : nullptr);
  if (keyField == nullptr) { return std::string(value); }
  RecordRef related;
  related.Open(static_cast<Integer>(target.id.Value()));
  related.Field(static_cast<Integer>(keyField->no.Value())).SetRange(Variant(Text<0>(value)));
  if (!static_cast<bool>(related.FindFirst())) { return std::string(value); }
  std::string caption;
  for (const FieldNo no : CaptionFieldsOf(target)) {
    const Variant held = related.Field(static_cast<Integer>(no.Value())).Value();
    Append(caption, std::string(std::string_view(held)));
  }
  return caption.empty() ? std::string(value) : caption;
}

std::string CaptionOfFilters(const void *record, const TableDef &table, std::string_view fields) {
  const RecordState *state = reinterpret_cast<const StateHandle *>(record)->Peek();
  std::string caption;
  for (const std::string_view name : NamesIn(fields)) {
    const FieldDef *def = FieldNamed(table, name);
    if (def == nullptr) { continue; }
    const std::optional<std::string> single = SingleFilterValue(state, def->no);
    if (!single.has_value() || single->empty()) { continue; }
    const std::optional<ResolvedRelation> resolved = ResolveRelation(record, table, *def);
    const TableEntry *entry = resolved.has_value() ? FindTable(resolved->table) : nullptr;
    if (entry == nullptr) {
      Append(caption, *single);
      continue;
    }
    const FieldDef *column = resolved->field.empty() ? nullptr : FieldNamed(*entry->table, resolved->field);
    Append(caption, CaptionOfRelated(*entry->table, column, *single));
  }
  return caption;
}

}

void CalcShownFlowFields(void *record, const TableDef &table, std::span<const ControlDef> controls) {
  const RecordState *state = reinterpret_cast<const StateHandle *>(record)->Peek();
  for (const ControlDef &control : controls) {
    if (control.kind == ControlKind::Field && control.field.Value() != 0) {
      static_cast<void>(CalcFieldIfCarried(record, table, state, control.field));
    }
    CalcShownFlowFields(record, table, control.children);
  }
}

std::string PageDataCaption(const void *record,
                            const TableDef &table,
                            PageType type,
                            std::string_view fields) {
  if (EntityOriented(type)) { return CaptionOfRecord(record, table); }
  return CaptionOfFilters(record, table, fields);
}

}
