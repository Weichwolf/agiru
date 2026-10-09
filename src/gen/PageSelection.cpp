#include "PageSelection.h"

#include "Apps.h"
#include "Ast.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"
#include "Expr.h"

#include <cstddef>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace agiru::gen {
namespace {

using SelectedParts = std::map<std::string, PageSelectionRow>;

std::string PartTarget(const al::PageControl &control) {
  std::string target;
  for (const auto &token : control.source) { target += token.text; }
  return LowerKey(target);
}

void SelectParts(std::vector<al::PageControl> &controls,
                 const Objects &objects,
                 SelectedParts &parts,
                 std::vector<PageSelectionRow> &rows) {
  std::erase_if(controls, [&](auto &control) {
    const auto found = objects.productExcludedPages.find(PartTarget(control));
    if (LowerKey(control.kind) == "part" && found != objects.productExcludedPages.end()) {
      if (objects.pages.contains(PartTarget(control))) {
        throw std::runtime_error("product selection: page is both selected and excluded: " +
                                 control.name);
      }
      const auto &target = found->second;
      PageSelectionRow row{.kind = "part",
                           .control = control.name,
                           .target = "Page " + std::to_string(target.id) + " " + target.name,
                           .targetSource = target.source,
                           .reason = target.reason,
                           .location = "layout",
                           .member = {}};
      if (!parts.emplace(LowerKey(control.name), row).second) {
        throw std::runtime_error("product selection: duplicate page control " + control.name);
      }
      rows.push_back(std::move(row));
      return true;
    }
    SelectParts(control.children, objects, parts, rows);
    return false;
  });
}

bool MemberPath(const al::Expr &expression, std::vector<std::string> &path) {
  const auto *walk = &expression;
  while (walk->kind == al::ExprKind::Binary && walk->text == ".") {
    if (path.size() == 3 || walk->children.size() != 2 ||
        walk->children.back().kind != al::ExprKind::Name) {
      return false;
    }
    path.insert(path.begin(), walk->children.back().text);
    walk = &walk->children.front();
  }
  if (walk->kind != al::ExprKind::Name) { return false; }
  path.insert(path.begin(), walk->text);
  return true;
}

const PageSelectionRow *ExcludedCall(const al::Stmt &statement, const SelectedParts &parts) {
  const auto &call = statement.expression;
  if (statement.kind != al::StmtKind::Expression || call.kind != al::ExprKind::Call ||
      call.children.empty()) {
    return nullptr;
  }
  std::vector<std::string> path;
  if (!MemberPath(call.children.front(), path) || path.size() != 4 ||
      LowerKey(path.front()) != "currpage" || LowerKey(path[2]) != "page") {
    return nullptr;
  }
  const auto found = parts.find(LowerKey(path[1]));
  return found == parts.end() ? nullptr : &found->second;
}

void NoteCalls(const std::vector<al::Stmt> &body,
               const SelectedParts &parts,
               const std::string &location,
               std::vector<PageSelectionRow> &rows) {
  std::size_t index = 0;
  for (const auto &statement : body) {
    const auto at = location + "/" + std::to_string(index++);
    if (const auto *part = ExcludedCall(statement, parts)) {
      auto row = *part;
      row.kind = "call";
      row.location = at;
      row.member = statement.expression.children.front().children.back().text;
      rows.push_back(std::move(row));
    }
    NoteCalls(statement.body, parts, at + "/body", rows);
    NoteCalls(statement.otherwise, parts, at + "/else", rows);
  }
}

void SelectProcedures(std::vector<al::ProcedureDecl> &procedures,
                      const SelectedParts &parts,
                      const std::string &location,
                      std::vector<PageSelectionRow> &rows) {
  for (auto &procedure : procedures) {
    NoteCalls(procedure.body, parts, location + "/" + procedure.name, rows);
  }
}

void SelectControlCalls(std::vector<al::PageControl> &controls,
                        const SelectedParts &parts,
                        const std::string &location,
                        std::vector<PageSelectionRow> &rows) {
  for (auto &control : controls) {
    const auto at = location + "/" + control.name;
    SelectProcedures(control.triggers, parts, at, rows);
    SelectControlCalls(control.children, parts, at, rows);
  }
}

}

void IndexProductPage(const TranspileScope &scope,
                      const std::filesystem::path &source,
                      const al::PageObject &page,
                      Objects &objects,
                      SourceDomain domain) {
  const auto reason = ProductExclusion(scope, source, domain);
  if (!reason) { return; }
  const auto origin =
      (domain == SourceDomain::SystemSymbols ? "system-symbols/" : "") + source.generic_string();
  const ProductPage declaration{.id = page.id,
                                .name = page.name,
                                .nameSpace = page.nameSpace,
                                .source = origin,
                                .reason = std::string(*reason)};
  for (const auto &name : {page.name, page.nameSpace + "." + page.name, std::to_string(page.id)}) {
    const auto [found, inserted] =
        objects.productExcludedPages.emplace(LowerKey(name), declaration);
    if (!inserted && found->second.source != declaration.source) {
      throw std::runtime_error("product selection: ambiguous excluded page " + name);
    }
  }
}

std::vector<PageSelectionRow> SelectProductPageParts(al::PageObject &page, Objects &objects) {
  SelectedParts parts;
  std::vector<PageSelectionRow> rows;
  SelectParts(page.layout, objects, parts, rows);
  if (parts.empty()) { return rows; }
  auto &excluded = objects.productExcludedParts[LowerKey(page.name)];
  for (const auto &[name, unused] : parts) { excluded.insert(name); }
  SelectProcedures(page.procedures, parts, "page", rows);
  SelectControlCalls(page.layout, parts, "layout", rows);
  SelectControlCalls(page.actions, parts, "actions", rows);
  SelectControlCalls(page.views, parts, "views", rows);
  return rows;
}

}
