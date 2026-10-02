#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

/// \file
/// \brief Session-independent AL error text and classification.

namespace agiru {

/// \brief An AL error owning its text and optional classification; no session or transaction state.
class Error : public std::runtime_error {
public:
  /// \brief Owns a message supplied as a C string or std::string.
  using std::runtime_error::runtime_error;

  /// \brief Owns a message supplied as a view.
  /// \param text The unchanged error text.
  explicit Error(std::string_view text) : std::runtime_error(std::string(text)) {}

  /// \brief Owns an error's text and classification.
  /// \param text The unchanged error text.
  /// \param code The classification; an empty code is interpreted by the runtime boundary.
  Error(std::string_view text, std::string_view code)
      : std::runtime_error(std::string(text)), code_(code) {}

  /// \brief Owns the message of an ErrorInfo-compatible value.
  /// \tparam Info A value providing `Message()`.
  /// \param info The error description.
  template <typename Info>
    requires requires(Info &info) { std::string_view{info.Message()}; }
  explicit Error(Info info) : std::runtime_error(std::string(info.Message())) {}

  /// \brief Reads the stored classification without applying a runtime default.
  /// \return The code, empty when none was supplied.
  [[nodiscard]] std::string_view Code() const { return code_; }

  /// \brief Copies the error, filling its classification only when it is empty.
  /// \param code The fallback classification.
  /// \return An independent error with unchanged text.
  [[nodiscard]] Error Coded(std::string_view code) const {
    return {what(), code_.empty() ? code : std::string_view(code_)};
  }

private:
  std::string code_;
};

}
