#include "runtime/Report.h"

#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "type/Variant.h"

#include "BuiltinsWritten.h"

#include <algorithm>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

std::vector<const ReportEntry *> &ReportEntries() {
  static std::vector<const ReportEntry *> entries;
  return entries;
}

std::once_flag &ReportsOnce() {
  static std::once_flag once;
  return once;
}

std::string Escaped(std::string_view text) {
  std::string out;
  out.reserve(text.size());
  for (const char c : text) {
    switch (c) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      default: out += c;
    }
  }
  return out;
}

constexpr std::int32_t kXmlFormat = 9;
constexpr int kFixedViewGroup = 2;

}

void RegisterReportEntry(const ReportEntry *entry) {
  ReportEntries().push_back(entry);
}

const ReportEntry *FindReport(ReportId id) {
  std::call_once(ReportsOnce(), [] {
    std::ranges::sort(ReportEntries(), [](const ReportEntry *a, const ReportEntry *b) {
      return a->id.Value() < b->id.Value();
    });
  });
  const auto found = std::lower_bound(
      ReportEntries().begin(),
      ReportEntries().end(),
      id.Value(),
      [](const ReportEntry *entry, auto number) { return entry->id.Value() < number; });
  if (found == ReportEntries().end() || (*found)->id != id) { return nullptr; }
  return *found;
}

void ReportDataset::Clear() {
  names_.clear();
  types_.clear();
  rows_.clear();
}

void ReportDataset::BeginRow() {
  rows_.emplace_back();
}

void ReportDataset::Add(std::string_view name, const Variant &value, std::string_view type) {
  if (rows_.empty()) { rows_.emplace_back(); }
  const auto known = std::ranges::find(names_, name);
  if (known == names_.end()) {
    names_.emplace_back(name);
    types_.emplace_back(type);
  }
  rows_.back().push_back(
      Column{.name = std::string(name), .text = std::string(Format(value, 0, kXmlFormat))});
}

void ReportDataset::EndRow() {}

std::string ReportDataset::Xml() const {
  std::string out = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n"
                    "<DataSet xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">\n";
  if (!names_.empty()) {
    out += "  <xs:schema>\n";
    for (std::size_t i = 0; i < names_.size(); ++i) {
      out += "    <xs:element name=\"" + Escaped(names_[i]) + "\" type=\"" + types_[i] + "\" />\n";
    }
    out += "  </xs:schema>\n";
  }
  for (const std::vector<Column> &row : rows_) {
    out += "  <Result>\n";
    for (const Column &column : row) {
      out += "    <" + column.name + ">" + Escaped(column.text) + "</" + column.name + ">\n";
    }
    out += "  </Result>\n";
  }
  out += "</DataSet>\n";
  return out;
}

void ReportDataset::WriteFile(std::string_view path) const {
  detail::WriteReportFile(path, Xml());
}

namespace detail {

void ApplyPageView(void *record, const TableDef &table, std::string_view view) {
  if (view.empty()) { return; }
  RecordState &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  std::vector<FieldFilter> kept;
  for (const FieldFilter &one : state.filters) {
    if (one.group != kFixedViewGroup) { kept.push_back(one); }
  }
  const Integer group = state.group;
  state.group = kFixedViewGroup;
  ApplyView(state, table, view);
  state.group = group;
  state.filters.insert(state.filters.end(), kept.begin(), kept.end());
}

void SeedNewPageRecord(void *record, const TableDef &table, std::string_view view, bool allFields) {
  ApplyPageView(record, table, view);
  SeedFromFilters(record, table, allFields);
}

void ApplyDataItemView(void *record, const TableDef &table, std::string_view view) {
  RecordState &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  std::vector<FieldFilter> kept;
  for (const FieldFilter &one : state.filters) {
    if (one.group != kFixedViewGroup) { kept.push_back(one); }
  }
  const Integer group = state.group;
  state.group = kFixedViewGroup;
  ApplyView(state, table, view);
  state.group = group;
  state.filters.insert(state.filters.end(), kept.begin(), kept.end());
}

void AdoptTableView(void *to, const void *from) {
  RecordState &into = reinterpret_cast<StateHandle *>(to)->Ensure();
  const RecordState *source = reinterpret_cast<const StateHandle *>(from)->Peek();
  if (source == nullptr) { return; }
  for (const FieldFilter &one : source->filters) {
    into.filters.push_back(
        FieldFilter{.field = one.field, .group = kFixedViewGroup, .text = one.text});
  }
}

void GiveRequestFilters(void *to, const void *from) {
  RecordState &into = reinterpret_cast<StateHandle *>(to)->Ensure();
  const RecordState *source = reinterpret_cast<const StateHandle *>(from)->Peek();
  into.filters = source == nullptr ? std::vector<FieldFilter>{} : source->filters;
}

void TakeRequestFilters(void *to, const void *from) {
  RecordState &into = reinterpret_cast<StateHandle *>(to)->Ensure();
  const RecordState *source = reinterpret_cast<const StateHandle *>(from)->Peek();
  std::erase_if(into.filters, [](const FieldFilter &one) { return one.group == 0; });
  if (source == nullptr) { return; }
  for (const FieldFilter &one : source->filters) {
    if (one.group == 0) { into.filters.push_back(one); }
  }
}

std::string ReportParametersXml(ReportId id, std::string_view name) {
  return "<?xml version=\"1.0\" standalone=\"yes\"?>\n<ReportParameters name=\"" + Escaped(name) +
         "\" id=\"" + std::to_string(id.Value()) +
         "\">\n  <Options />\n  <DataItems />\n</ReportParameters>\n";
}

void WriteReportFile(std::string_view path, std::string_view text) {
  std::ofstream out{std::string(path), std::ios::binary | std::ios::trunc};
  if (!out) { throw Error("The report file " + std::string(path) + " cannot be written"); }
  out << text;
}

}

}
