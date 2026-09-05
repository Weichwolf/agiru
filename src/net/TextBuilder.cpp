#include "type/TextBuilder.h"

#include "type/Boolean.h"
#include "type/Integer.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>

namespace agiru {

namespace {

std::size_t At(Integer position, std::size_t length) {
  if (position < 1) { return 0; }
  return std::min(static_cast<std::size_t>(position) - 1, length);
}

std::size_t Span(Integer count, std::size_t from, std::size_t length) {
  if (count < 0) { return 0; }
  return std::min(static_cast<std::size_t>(count), length - from);
}

}

::agiru::Boolean TextBuilder::Append(std::string_view Text) {
  text_.append(Text);
  return true;
}

::agiru::Boolean TextBuilder::AppendLine() {
  text_ += '\n';
  return true;
}

::agiru::Boolean TextBuilder::AppendLine(std::string_view Text) {
  text_.append(Text);
  return AppendLine();
}

::agiru::Integer TextBuilder::Capacity() { return static_cast<Integer>(text_.capacity()); }

::agiru::Integer TextBuilder::Capacity(::agiru::Integer NewCapacity) {
  const Integer was = Capacity();
  if (NewCapacity > 0) { text_.reserve(static_cast<std::size_t>(NewCapacity)); }
  return was;
}

void TextBuilder::Clear() { text_.clear(); }

::agiru::Boolean TextBuilder::EnsureCapacity(::agiru::Integer NewCapacity) {
  if (NewCapacity > 0) { text_.reserve(static_cast<std::size_t>(NewCapacity)); }
  return true;
}

::agiru::Boolean TextBuilder::Insert(::agiru::Integer Position, std::string_view Text) {
  text_.insert(At(Position, text_.size()), Text);
  return true;
}

::agiru::Integer TextBuilder::Length(::agiru::Integer NewLength) {
  const auto was = static_cast<Integer>(text_.size());
  if (NewLength > 0 || (NewLength == 0 && was != 0)) {
    text_.resize(static_cast<std::size_t>(std::max(NewLength, 0)), '\0');
  }
  return was;
}

::agiru::Integer TextBuilder::MaxCapacity() { return std::numeric_limits<Integer>::max(); }

::agiru::Boolean TextBuilder::Remove(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  const std::size_t from = At(StartIndex, text_.size());
  text_.erase(from, Span(Count, from, text_.size()));
  return true;
}

::agiru::Boolean TextBuilder::Replace(std::string_view OldText,
                                      std::string_view NewText,
                                      ::agiru::Integer StartIndex,
                                      ::agiru::Integer Count) {
  if (OldText.empty()) { return true; }
  const std::size_t from = At(StartIndex, text_.size());
  const std::size_t span = Span(Count, from, text_.size());
  std::string within = text_.substr(from, span);
  for (std::size_t at = within.find(OldText); at != std::string::npos;
       at = within.find(OldText, at + NewText.size())) {
    within.replace(at, OldText.size(), NewText);
  }
  text_.replace(from, span, within);
  return true;
}

::agiru::Boolean TextBuilder::Replace(std::string_view OldText, std::string_view NewText) {
  return Replace(OldText, NewText, 1, static_cast<Integer>(text_.size()));
}

std::string TextBuilder::ToText() { return text_; }

std::string TextBuilder::ToText(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  const std::size_t from = At(StartIndex, text_.size());
  return text_.substr(from, Span(Count, from, text_.size()));
}

}
