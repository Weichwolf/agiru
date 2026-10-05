#include "Refused.h"

#include "Ast.h"
#include "EnumWriter.h"
#include "NativeMethods.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>

namespace agiru::gen {
namespace {

std::string DeclaredType(const al::VarDecl &declared) {
  std::string out;
  if (!declared.dimensions.empty()) {
    out = "array[";
    for (const int dimension : declared.dimensions) {
      if (out.back() != '[') { out += ','; }
      out += std::to_string(dimension);
    }
    out += "] of ";
  }
  out += declared.type;
  if (!declared.subtype.empty()) { out += " " + declared.subtype; }
  if (declared.length != 0) { out += "[" + std::to_string(declared.length) + "]"; }
  if (!declared.arguments.empty()) {
    out += " of [";
    for (const auto &argument : declared.arguments) {
      if (out.back() != '[') { out += ", "; }
      out += DeclaredType(argument);
    }
    out += ']';
  }
  for (const auto &member : declared.members) { out += " \"" + member + "\""; }
  if (declared.temporary) { out += " temporary"; }
  return out;
}

constexpr std::array kRefused{
    std::string_view{"abouttextml"},
    std::string_view{"abouttitleml"},
    std::string_view{"additionalsearchtermsml"},
    std::string_view{"captionml"},
    std::string_view{"columnstoreindex"},
    std::string_view{"customactiontype"},
    std::string_view{"entitycaptionml"},
    std::string_view{"entitysetcaptionml"},
    std::string_view{"flowcaption"},
    std::string_view{"flowtemplate"},
    std::string_view{"flowtemplategallery"},
    std::string_view{"instructionaltextml"},
    std::string_view{"linkedintransaction"},
    std::string_view{"linkedobject"},
    std::string_view{"navigationpageid"},
    std::string_view{"optioncaptionml"},
    std::string_view{"papersourcedefaultpage"},
    std::string_view{"papersourcefirstpage"},
    std::string_view{"papersourcelastpage"},
    std::string_view{"pdffontembedding"},
    std::string_view{"profiledescriptionml"},
    std::string_view{"promotedactioncategoriesml"},
    std::string_view{"requestfilterheadingml"},
    std::string_view{"signdisplacement"},
    std::string_view{"sqldatatype"},
    std::string_view{"sqlindex"},
    std::string_view{"summaryml"},
    std::string_view{"testtablerelation"},
    std::string_view{"title"},
    std::string_view{"tooltipml"},
    std::string_view{"externalname"},
    std::string_view{"externaltype"},
    std::string_view{"externalaccess"},
    std::string_view{"publickeytoken"},
    std::string_view{"enableexternalassemblies"},
};

constexpr std::array kRefusedValue{
    std::pair{std::string_view{"scope"}, std::string_view{"onprem"}},
    std::pair{std::string_view{"tabletype"}, std::string_view{"crm"}},
    std::pair{std::string_view{"tabletype"}, std::string_view{"cds"}},
    std::pair{std::string_view{"tabletype"}, std::string_view{"externalsql"}},
    std::pair{std::string_view{"tabletype"}, std::string_view{"exchange"}},
    std::pair{std::string_view{"tabletype"}, std::string_view{"microsoftgraph"}},
};

}

bool RefusedByName(std::string_view name) {
  return std::ranges::find(kRefused, name) != kRefused.end();
}

std::string NativeMethodIdentity(const al::CodeunitObject &codeunit,
                                 const al::ProcedureDecl &procedure) {
  std::string out = "codeunit " + std::to_string(codeunit.id) + " ";
  if (!codeunit.nameSpace.empty()) { out += codeunit.nameSpace + '.'; }
  out += codeunit.name + '.' + procedure.name + '(';
  for (const auto &parameter : procedure.parameters) {
    if (out.back() != '(') { out += "; "; }
    if (parameter.byReference) { out += "var "; }
    out += parameter.name + ": " + DeclaredType(parameter);
  }
  out += ')';
  if (!procedure.returnType.empty()) {
    if (!procedure.returnName.empty()) { out += ' ' + procedure.returnName; }
    out += ": " + DeclaredType(procedure.returned);
  }
  return out;
}

void CollectRefused(const std::vector<al::Property> &properties,
                    std::string_view where,
                    std::vector<RefusedProperty> &into) {
  for (const al::Property &property : properties) {
    const std::string key = LowerKey(property.name);
    if (key == "linkedobject" && LowerKey(property.text) == "false") { continue; }
    if (std::ranges::find(kRefused, key) == kRefused.end()) {
      const std::string value = LowerKey(property.text);
      bool named = false;
      for (const auto &[name, only] : kRefusedValue) {
        named = named || (key == name && value == only);
      }
      if (!named) { continue; }
    }
    into.push_back(RefusedProperty{.property = property.name, .where = std::string(where)});
  }
}

namespace {

void Walk(const std::vector<al::PageControl> &controls,
          const std::string &where,
          std::vector<RefusedProperty> &into) {
  for (const al::PageControl &control : controls) {
    CollectRefused(control.properties, where + " control " + control.name, into);
    Walk(control.children, where, into);
  }
}

}

std::vector<RefusedProperty> Refused(const al::TableObject &table) {
  std::vector<RefusedProperty> found;
  CollectRefused(table.properties, "table " + table.name, found);
  for (const al::FieldDecl &field : table.fields) {
    CollectRefused(field.properties, "table " + table.name + " field " + field.name, found);
  }
  for (const al::FieldDecl &field : table.modified) {
    CollectRefused(field.properties, "table " + table.name + " field " + field.name, found);
  }
  for (const al::KeyDecl &key : table.keys) {
    CollectRefused(key.properties, "table " + table.name + " key " + key.name, found);
  }
  return found;
}

std::vector<RefusedProperty> Refused(const al::PageObject &page) {
  std::vector<RefusedProperty> found;
  const std::string where = "page " + page.name;
  CollectRefused(page.properties, where, found);
  for (const auto &layout : page.rendering) {
    const std::string owner = layout.owner.extension ? "reportextension " : "report ";
    CollectRefused(layout.properties, owner + layout.owner.name + " layout " + layout.name, found);
  }
  Walk(page.layout, where, found);
  Walk(page.actions, where, found);
  return found;
}

std::vector<RefusedProperty> Refused(const al::CodeunitObject &codeunit) {
  std::vector<RefusedProperty> found;
  CollectRefused(codeunit.properties, "codeunit " + codeunit.name, found);
  for (const auto &procedure : codeunit.procedures) {
    if (!al::HasAttribute(procedure, "Native") || BindNativeMethod(codeunit, procedure)) {
      continue;
    }
    found.push_back({.property = "Native", .where = NativeMethodIdentity(codeunit, procedure)});
  }
  return found;
}

std::vector<RefusedProperty> Refused(const al::EnumObject &declared) {
  std::vector<RefusedProperty> found;
  CollectRefused(declared.properties, "enum " + declared.name, found);
  for (const al::EnumValueDecl &value : declared.values) {
    CollectRefused(value.properties, "enum " + declared.name + " value " + value.name, found);
  }
  return found;
}

}
