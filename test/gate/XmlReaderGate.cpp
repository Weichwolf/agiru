#include "dotnet/XmlReader.h"
#include "runtime/Error.h"

#include "Check.h"

#include <string>
#include <vector>

using agiru::dotnet::StringReader;
using agiru::dotnet::XmlNodeType;
using agiru::dotnet::XmlReader;
using agiru::dotnet::XmlReaderSettings;

namespace {

/// A READER WALKS THE NODES FORWARD, AS .NET'S DOES, with the node types numbered the same:
/// `XML Buffer Writer` fills the XML Buffer from `Read`, `NodeType`, `Depth`, `Name`, `Value` and the
/// attribute moves, and the whitespace between elements is a node like in .NET (board:0676).
void AReaderWalksElementsAttributesAndText() {
  StringReader source;
  source = source.StringReader("<?xml version=\"1.0\"?><a x=\"1\"><b>hi</b><c/></a>");
  XmlReaderSettings settings;
  settings = settings.XmlReaderSettings();
  settings.DtdProcessing(agiru::dotnet::DtdProcessing::Ignore());
  XmlReader reader = XmlReader::Create(source, settings);
  std::vector<std::string> walk;
  while (reader.Read()) {
    walk.push_back(std::to_string(reader.NodeType().Number()) + ":" + std::string(reader.Name().Value()) +
                   "@" + std::to_string(reader.Depth()));
    if (reader.NodeType().Equals(XmlNodeType::Element()) && reader.MoveToFirstAttribute()) {
      do {
        walk.push_back("attr " + std::string(reader.Name().Value()) + "=" + std::string(reader.Value().Value()));
      } while (reader.MoveToNextAttribute());
    }
  }
  CHECK_TRUE("the walk has the declaration, the elements, the text and the ends", walk.size() >= 7);
  CHECK_TEXT("the first node is the declaration", walk[0], "17:xml@0");
  CHECK_TEXT("then the root element", walk[1], "1:a@0");
  CHECK_TEXT("with its attribute", walk[2], "attr x=1");
  CHECK_TEXT("then b one deeper", walk[3], "1:b@1");
  CHECK_TEXT("its text two deeper", walk[4], "3:#text@2");
  CHECK_TEXT("its end", walk[5], "15:b@1");
  CHECK_TRUE("and the walk ends with the reader at its end", reader.Eof());
  std::string said;
  try {
    StringReader bad;
    bad = bad.StringReader("<a><b></a>");
    XmlReader broken = XmlReader::Create(bad, settings);
    while (broken.Read()) {}
  } catch (const agiru::Error &e) { said = e.what(); }
  CHECK_TRUE("malformed XML refuses when it is reached", said.find("well-formed") != std::string::npos);
}

} // namespace

int main() {
  return gate::Run("XmlReader", [] { AReaderWalksElementsAttributesAndText(); });
}
