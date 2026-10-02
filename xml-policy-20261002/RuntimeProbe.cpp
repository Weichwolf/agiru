#include "dotnet/XmlDocument.h"
#include "dotnet/XmlReader.h"
#include "runtime/ErrorValue.h"
#include "type/Blob.h"
#include "type/Stream.h"

#include <cstdio>
#include <string>
#include <string_view>

namespace {

std::string Utf16(std::string_view text) {
  std::string bytes{"\xff\xfe", 2};
  for (const char byte : text) {
    bytes += byte;
    bytes += '\0';
  }
  return bytes;
}

void Measure(
    std::string_view name, std::string_view text, int mode, bool utf16, bool load, bool closed) {
  agiru::dotnet::XmlReaderSettings settings;
  settings.DtdProcessing(mode);
  agiru::Blob bytes;
  auto output = bytes.CreateOutStream();
  output.WriteBytes(utf16 ? Utf16(text) : std::string(text));
  auto source = bytes.CreateInStream();
  std::string values;
  std::string inherited;
  bool refused = false;
  bool doctype = false;
  bool eof = false;
  try {
    auto reader = agiru::dotnet::XmlReader::Create(source, settings);
    if (closed) { reader.Close(); }
    if (load) {
      agiru::dotnet::XmlDocument document;
      document.Load(reader);
      values = document.DocumentElement().InnerText().Value();
      inherited = document.DocumentElement().GetAttribute("inherited").Value();
      doctype = !document.DocumentType().IsNullObject();
    } else {
      while (reader.Read()) {
        doctype |= reader.NodeType().Number() == 10;
        values += reader.Value().Value();
        if (reader.MoveToFirstAttribute()) {
          do {
            if (reader.Name().Value() == "inherited") { inherited = reader.Value().Value(); }
          } while (reader.MoveToNextAttribute());
          static_cast<void>(reader.MoveToElement());
        }
      }
    }
    eof = reader.Eof();
  } catch (const agiru::Error &) { refused = true; }
  const bool external = values.find("AGIRU-OWNED-XML-POLICY-MARKER") != std::string::npos;
  const bool internal = values.find("internal-value") != std::string::npos;
  std::printf("{\"case\":\"%.*s\",\"mode\":%d,\"utf16\":%s,\"document_load\":%s,"
              "\"closed\":%s,\"refused\":%s,\"doctype_reported\":%s,\"eof\":%s,"
              "\"owned_external_marker\":%s,\"internal_entity_expanded\":%s,"
              "\"dtd_default_attribute\":%s}\n",
              static_cast<int>(name.size()),
              name.data(),
              mode,
              utf16 ? "true" : "false",
              load ? "true" : "false",
              closed ? "true" : "false",
              refused ? "true" : "false",
              doctype ? "true" : "false",
              eof ? "true" : "false",
              external ? "true" : "false",
              internal ? "true" : "false",
              inherited == "DTD-DEFAULT" ? "true" : "false");
}

void Positioned(bool utf16) {
  constexpr std::string_view input = "<root><first>one</first><last>two</last></root>";
  agiru::Blob bytes;
  auto output = bytes.CreateOutStream();
  output.WriteBytes(utf16 ? Utf16(input) : std::string(input));
  auto source = bytes.CreateInStream();
  auto reader = agiru::dotnet::XmlReader::Create(source);
  bool reached = false;
  while (reader.Read()) {
    if (reader.NodeType().Number() == 1 && reader.Name().Value() == "last") {
      reached = true;
      break;
    }
  }
  agiru::dotnet::XmlDocument document;
  document.Load(reader);
  std::printf("{\"case\":\"positioned-last\",\"mode\":0,\"utf16\":%s,\"document_load\":true,"
              "\"closed\":false,\"refused\":false,\"doctype_reported\":false,"
              "\"owned_external_marker\":false,\"internal_entity_expanded\":false,"
              "\"dtd_default_attribute\":false,\"reached_last\":%s,\"reloads_original_root\":%s,"
              "\"reloads_already_consumed_first\":%s}\n",
              utf16 ? "true" : "false",
              reached ? "true" : "false",
              document.DocumentElement().Name().Value() == "root" ? "true" : "false",
              document.DocumentElement().InnerText().Value() == "onetwo" ? "true" : "false");
}

}

int main(int argc, char **argv) {
  if (argc != 2) { return 2; }
  const std::string general = "<!DOCTYPE root [<!ENTITY own SYSTEM 'file://" +
                              std::string(argv[1]) + "/owned-entity.txt'>]><root>&own;</root>";
  constexpr std::string_view internal = "<!DOCTYPE root [<!ENTITY inner 'internal-value'><!ATTLIST "
                                        "root inherited CDATA 'DTD-DEFAULT'>]><root>&inner;</root>";
  for (const bool utf16 : {false, true}) {
    for (const int mode : {0, 1, 2}) {
      for (const bool load : {false, true}) {
        Measure("general", general, mode, utf16, load, false);
        Measure("internal", internal, mode, utf16, load, false);
      }
      Measure("closed-plain", "<root>plain</root>", mode, utf16, true, true);
    }
    Positioned(utf16);
  }
}
