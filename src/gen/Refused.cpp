#include "Refused.h"

#include "EnumWriter.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>

namespace agiru::gen {
namespace {

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

void CollectRefused(const std::vector<al::Property> &properties,
                    std::string_view where,
                    std::vector<RefusedProperty> &into) {
  for (const al::Property &property : properties) {
    const std::string key = LowerKey(property.name);
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
  Walk(page.layout, where, found);
  Walk(page.actions, where, found);
  return found;
}

std::vector<RefusedProperty> Refused(const al::CodeunitObject &codeunit) {
  std::vector<RefusedProperty> found;
  CollectRefused(codeunit.properties, "codeunit " + codeunit.name, found);
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
