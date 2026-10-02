#pragma once

#include "type/Integer.h"

namespace agiru::dotnet {

/// \brief Immutable Int32 getters from
/// Microsoft.Dynamics.Nav.Runtime.Designer.DesignerFieldProperty. Values are the original
/// Ncl 28.4.53241.0 getter constants, not AL field ordinals or names. Assembly SHA256
/// d37240e842d6e259407f27fc100cccc6fd4d885e9dd059503253b516491b4213; complete signatures and IL
/// evidence are recorded in WI 0035.
class DesignerFieldProperty {
public:
  /// \brief The BlankZero property ID. \return 46.
  [[nodiscard]] static constexpr ::agiru::Integer BlankZero() { return kBlankZero; }

  /// \brief The Caption property ID. \return 4.
  [[nodiscard]] static constexpr ::agiru::Integer Caption() { return kCaption; }

  /// \brief The DecimalPlaces property ID. \return 57.
  [[nodiscard]] static constexpr ::agiru::Integer DecimalPlaces() { return kDecimalPlaces; }

  /// \brief The Description property ID. \return 2.
  [[nodiscard]] static constexpr ::agiru::Integer Description() { return kDescription; }

  /// \brief The Editable property ID. \return 68.
  [[nodiscard]] static constexpr ::agiru::Integer Editable() { return kEditable; }

  /// \brief The Enabled property ID. \return 67.
  [[nodiscard]] static constexpr ::agiru::Integer Enabled() { return kEnabled; }

  /// \brief The InitValue property ID. \return 63.
  [[nodiscard]] static constexpr ::agiru::Integer InitValue() { return kInitValue; }

  /// \brief The MultiLine property ID. \return 130.
  [[nodiscard]] static constexpr ::agiru::Integer MultiLine() { return kMultiLine; }

  /// \brief The OptionString property ID. \return 75.
  [[nodiscard]] static constexpr ::agiru::Integer OptionString() { return kOptionString; }

  /// \brief The ShowCaption property ID. \return 129.
  [[nodiscard]] static constexpr ::agiru::Integer ShowCaption() { return kShowCaption; }

private:
  static constexpr ::agiru::Integer kBlankZero = 46;
  static constexpr ::agiru::Integer kCaption = 4;
  static constexpr ::agiru::Integer kDecimalPlaces = 57;
  static constexpr ::agiru::Integer kDescription = 2;
  static constexpr ::agiru::Integer kEditable = 68;
  static constexpr ::agiru::Integer kEnabled = 67;
  static constexpr ::agiru::Integer kInitValue = 63;
  static constexpr ::agiru::Integer kMultiLine = 130;
  static constexpr ::agiru::Integer kOptionString = 75;
  static constexpr ::agiru::Integer kShowCaption = 130;
};

}
