#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru {

/// \brief Owned PostgreSQL connection options parsed by libpq, without applying defaults.
class ConnectionInfo {
public:
  /// \brief Parses URI or keyword syntax, including escaped values and repeated options.
  /// \param connection The supplied connection string, never reproduced in diagnostics.
  /// \throws DatabaseError for invalid syntax, embedded NUL or an absent/empty database name.
  explicit ConnectionInfo(std::string_view connection);

  /// \brief The decoded, explicitly supplied database name.
  /// \return A view valid while this object remains unchanged and alive.
  [[nodiscard]] std::string_view Database() const { return database_; }

  /// \brief Serializes the options with exactly one replacement database setting.
  /// \param database The literal replacement name, not a connection string.
  /// \return Canonical keyword syntax preserving every other supplied option.
  /// \throws DatabaseError for an empty name or embedded NUL.
  [[nodiscard]] std::string AtDatabase(std::string_view database) const;

private:
  std::vector<std::pair<std::string, std::string>> options_;
  std::string database_;
};

}
