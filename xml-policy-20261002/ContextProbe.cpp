#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include <libxml/SAX2.h>
#include <libxml/entities.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

namespace {

struct Policy {
  bool prohibit = false;
  bool denied = false;
  int resolverCalls = 0;
  int externalEntityCalls = 0;
};

Policy &State(void *context) {
  return *static_cast<Policy *>(static_cast<xmlParserCtxtPtr>(context)->_private);
}

void InternalSubset(void *context,
                    const xmlChar *name,
                    const xmlChar *publicId,
                    const xmlChar *systemId) {
  auto &policy = State(context);
  if (policy.prohibit) {
    policy.denied = true;
    xmlStopParser(static_cast<xmlParserCtxtPtr>(context));
    return;
  }
  xmlSAX2InternalSubset(context, name, publicId, systemId);
}

xmlParserInputPtr DenyResource(void *context, const xmlChar *, const xmlChar *) {
  auto &policy = State(context);
  ++policy.resolverCalls;
  policy.denied = true;
  xmlStopParser(static_cast<xmlParserCtxtPtr>(context));
  return nullptr;
}

xmlEntityPtr CheckEntity(void *context, xmlEntityPtr entity) {
  if (entity != nullptr && (entity->etype == XML_EXTERNAL_GENERAL_PARSED_ENTITY ||
                            entity->etype == XML_EXTERNAL_GENERAL_UNPARSED_ENTITY ||
                            entity->etype == XML_EXTERNAL_PARAMETER_ENTITY)) {
    auto &policy = State(context);
    ++policy.externalEntityCalls;
    policy.denied = true;
    xmlStopParser(static_cast<xmlParserCtxtPtr>(context));
    return nullptr;
  }
  return entity;
}

xmlEntityPtr GeneralEntity(void *context, const xmlChar *name) {
  return CheckEntity(context, xmlSAX2GetEntity(context, name));
}

xmlEntityPtr ParameterEntity(void *context, const xmlChar *name) {
  return CheckEntity(context, xmlSAX2GetParameterEntity(context, name));
}

void Quiet(void *, xmlErrorPtr) {}

std::string Utf16(std::string_view text) {
  std::string bytes{"\xff\xfe", 2};
  for (const char byte : text) {
    bytes += byte;
    bytes += '\0';
  }
  return bytes;
}

bool Measure(
    std::string_view name, std::string_view text, bool guarded, bool prohibit, bool utf16) {
  const std::string bytes = utf16 ? Utf16(text) : std::string(text);
  const std::unique_ptr<xmlParserCtxt, decltype(&xmlFreeParserCtxt)> context(xmlNewParserCtxt(),
                                                                             &xmlFreeParserCtxt);
  if (!context) { return false; }
  Policy policy{.prohibit = prohibit};
  context->_private = &policy;
  context->sax->serror = &Quiet;
  if (guarded) {
    context->sax->internalSubset = &InternalSubset;
    context->sax->resolveEntity = &DenyResource;
    context->sax->getEntity = &GeneralEntity;
    context->sax->getParameterEntity = &ParameterEntity;
  }
  const std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> document(
      xmlCtxtReadMemory(context.get(),
                        bytes.data(),
                        static_cast<int>(bytes.size()),
                        nullptr,
                        nullptr,
                        XML_PARSE_NONET | XML_PARSE_NOENT | XML_PARSE_DTDLOAD | XML_PARSE_DTDATTR),
      &xmlFreeDoc);
  std::string value;
  std::string attribute;
  if (document) {
    xmlNodePtr root = xmlDocGetRootElement(document.get());
    if (root != nullptr) {
      if (xmlChar *held = xmlNodeGetContent(root); held != nullptr) {
        value = reinterpret_cast<const char *>(held);
        xmlFree(held);
      }
      if (xmlChar *held = xmlGetProp(root, reinterpret_cast<const xmlChar *>("inherited"));
          held != nullptr) {
        attribute = reinterpret_cast<const char *>(held);
        xmlFree(held);
      }
    }
  }
  const bool leaked = value.find("AGIRU-OWNED-XML-POLICY-MARKER") != std::string::npos;
  std::printf("{\"case\":\"%.*s\",\"guarded\":%s,\"prohibit\":%s,\"utf16\":%s,"
              "\"document\":%s,\"well_formed\":%s,\"error_code\":%d,\"denied\":%s,"
              "\"resource_calls\":%d,\"entity_calls\":%d,"
              "\"owned_external_marker\":%s,\"internal_entity_expanded\":%s,\"dtd_default_"
              "attribute\":%s}\n",
              static_cast<int>(name.size()),
              name.data(),
              guarded ? "true" : "false",
              prohibit ? "true" : "false",
              utf16 ? "true" : "false",
              document ? "true" : "false",
              context->wellFormed ? "true" : "false",
              context->errNo,
              policy.denied ? "true" : "false",
              policy.resolverCalls,
              policy.externalEntityCalls,
              leaked ? "true" : "false",
              value == "internal-value" ? "true" : "false",
              attribute == "DTD-DEFAULT" ? "true" : "false");
  if (!guarded) { return name == "internal" ? document != nullptr : leaked; }
  return prohibit || name != "internal" ? policy.denied && !leaked
                                        : document && value == "internal-value";
}

}

int main(int argc, char **argv) {
  if (argc != 2 && argc != 3) { return 2; }
  const std::string directory = "file://" + std::string(argv[1]);
  const std::string general = "<!DOCTYPE root [<!ENTITY own SYSTEM '" + directory +
                              "/owned-entity.txt'>]><root>&own;</root>";
  if (argc == 3) {
    const std::string_view mode(argv[2]);
    if (mode != "guarded" && mode != "prohibit" && mode != "unsafe") { return 2; }
    return Measure("general", general, mode != "unsafe", mode == "prohibit", false) ? 0 : 1;
  }
  const std::string parameter = "<!DOCTYPE root [<!ENTITY % own SYSTEM '" + directory +
                                "/owned-dtd.txt'>%own;]><root>&nested;</root>";
  const std::string subset =
      "<!DOCTYPE root SYSTEM '" + directory + "/owned-dtd.txt'><root>&nested;</root>";
  constexpr std::string_view internal = "<!DOCTYPE root [<!ENTITY inner 'internal-value'><!ATTLIST "
                                        "root inherited CDATA 'DTD-DEFAULT'>]><root>&inner;</root>";
  bool good = true;
  for (const bool utf16 : {false, true}) {
    for (const bool guarded : {false, true}) {
      good &= Measure("general", general, guarded, false, utf16);
      good &= Measure("parameter", parameter, guarded, false, utf16);
      good &= Measure("external-subset", subset, guarded, false, utf16);
      good &= Measure("internal", internal, guarded, false, utf16);
    }
    good &= Measure("general", general, true, true, utf16);
    good &= Measure("parameter", parameter, true, true, utf16);
    good &= Measure("external-subset", subset, true, true, utf16);
    good &= Measure("internal", internal, true, true, utf16);
  }
  return good ? 0 : 1;
}
