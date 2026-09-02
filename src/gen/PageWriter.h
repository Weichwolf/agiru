#pragma once

#include "Ast.h"
#include "CodeunitWriter.h"

#include <string>

namespace agiru::gen {

/// \brief What one translated page produced.
struct PageHeader {
  std::string text;
  DotNetUse dotnet;
  DotNetUse absent;
};

/// \brief Translates an AL page into a C++ class.
///
/// \param object   The parsed page.
/// \param source   The `.al` path, for the provenance header.
/// \param objects  Everything the run has translated so far.
/// \return The header.
///
/// \note A CONTROL BECOMES A MEMBER AND THE TREE IS FLATTENED, because that is how AL reaches one:
///       `SalesOrder."No.".SetValue('X')` names the control and never the group it sits in. The
///       nesting decides the LAYOUT, which is a property of the running page rather than of its
///       declaration, so it is carried by the control table and not by the class.
PageHeader
WritePage(const al::PageObject &object, const std::string &source, const Objects &objects);

/// \brief Where a page's header goes, relative to an app root.
/// \param object The parsed page.
/// \return The path.
std::string PageHeaderPath(const al::PageObject &object);

} // namespace agiru::gen
