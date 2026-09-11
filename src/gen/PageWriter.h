#pragma once

#include "Ast.h"
#include "CodeunitWriter.h"
#include "Scope.h"

#include <map>
#include <string>

namespace agiru::gen {

struct PageHeader {
  std::string text;
  DotNetUse dotnet;
  DotNetUse absent;
};

PageHeader
WritePage(const al::PageObject &object, const std::string &source, const Objects &objects);

std::string PageHeaderPath(const al::PageObject &object);

[[nodiscard]] ObjectKind PageKind(const al::PageObject &object);

void PrepareReport(al::PageObject &report);

void PrepareXmlPort(al::PageObject &port);

[[nodiscard]] std::vector<const al::PageControl *> DataItemsOf(const al::PageObject &report);

[[nodiscard]] const al::VarDecl *DataItemVariable(const al::PageObject &report,
                                                  std::string_view name);

[[nodiscard]] std::map<std::string, std::string>
WithDataItems(std::map<std::string, std::string> named, const al::PageObject &report);

[[nodiscard]] std::map<std::string, std::string>
WithElements(std::map<std::string, std::string> named, const al::PageObject &port);

void SynthesizeRunObjectActions(al::PageObject &page, const Objects &objects);

std::string
PageDefinition(const al::PageObject &page, const Objects &objects, const al::TableObject *source);

[[nodiscard]] std::map<std::string, std::string> ControlIdentifiers(const al::PageObject &object);

[[nodiscard]] std::map<std::string, std::string> PartPages(const al::PageObject &object);

[[nodiscard]] std::string PageVariableIdentifier(const al::PageObject &page, std::string_view name);

[[nodiscard]] std::vector<al::VarDecl> VariablesAside(const al::PageObject &page);

[[nodiscard]] std::map<std::string, std::string> ControlIdentifiers(const al::PageObject &object,
                                                                    const Objects &objects);

[[nodiscard]] std::string ControlIdentifier(const std::map<std::string, std::string> &named,
                                            std::string_view alName);

}
