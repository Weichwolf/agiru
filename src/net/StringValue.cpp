#include "type/StringValue.h"

#include "type/Char.h"
#include "type/Integer.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace agiru {

Char StringValue::operator[](Integer index) const {
  if (index < 1) {
    throw StringError("the string index " + std::to_string(index) +
                      " is below one, and AL counts from one");
  }
  return Char{detail::CodePointAt(Stored(), static_cast<std::size_t>(index) - 1)};
}

std::string detail::ReplaceTextCharacter(std::string_view text, Integer index, Char character) {
  constexpr Integer kLastChar = 0xFFFF;
  constexpr Integer kFirstSurrogate = 0xD800;
  constexpr Integer kLastSurrogate = 0xDFFF;
  const Integer code = character.AsInteger();
  if (code < 0 || code > kLastChar) {
    throw StringError("an AL Char must be in the range 0..65535");
  }
  if (code >= kFirstSurrogate && code <= kLastSurrogate) {
    throw StringError("isolated UTF-16 surrogate character writes are not implemented");
  }
  const std::size_t length = Utf16Length(text);
  if (index < 1 || static_cast<std::size_t>(index) > length + 1) {
    throw StringError("Index " + std::to_string(index) + " is outside the text of length " +
                      std::to_string(length));
  }
  const auto unit = static_cast<std::size_t>(index);
  if (unit <= length) { static_cast<void>(CodePointAt(text, unit - 1)); }
  const std::size_t from = ByteOfUnit(text, unit);
  const std::size_t upto = ByteOfUnit(text, unit + 1);
  std::string out(text);
  out.replace(from, upto - from, Encoded(character));
  return out;
}

}
