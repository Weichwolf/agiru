#include "type/Blob.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace agiru {

Blob::Blob(const Blob &other)
    : bytes_(other.bytes_ == nullptr ? nullptr : std::make_shared<Storage>(*other.bytes_)) {}

Blob &Blob::operator=(const Blob &other) {
  if (this != &other) {
    bytes_ = other.bytes_ == nullptr ? nullptr : std::make_shared<Storage>(*other.bytes_);
  }
  return *this;
}

std::shared_ptr<Blob::Storage> Blob::Pin() const {
  if (bytes_ == nullptr) { bytes_ = std::make_shared<Storage>(); }
  return bytes_;
}

std::size_t Blob::Length() const {
  return bytes_ == nullptr ? 0 : bytes_->size();
}

const std::vector<std::uint8_t> &Blob::Bytes() const {
  static const Storage empty;
  return bytes_ == nullptr ? empty : *bytes_;
}

void Blob::Set(std::vector<std::uint8_t> bytes) {
  *Pin() = std::move(bytes);
}

bool Blob::operator==(const Blob &other) const {
  return Bytes() == other.Bytes();
}

}
