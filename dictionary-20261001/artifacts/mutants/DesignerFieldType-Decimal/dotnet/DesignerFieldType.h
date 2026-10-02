#pragma once

#include "type/Integer.h"

namespace agiru::dotnet {

/// \brief Immutable Int32 getters from Microsoft.Dynamics.Nav.Runtime.Designer.DesignerFieldType.
/// Values are the original Ncl 28.4.53241.0 getter constants, not a computed AL field type offset.
/// Assembly SHA256 d37240e842d6e259407f27fc100cccc6fd4d885e9dd059503253b516491b4213;
/// complete signatures and IL evidence are recorded in WI 0035.
class DesignerFieldType {
public:
  /// \brief The BigInteger designer type ID. \return 36096.
  [[nodiscard]] static constexpr ::agiru::Integer BigInteger() { return kBigInteger; }

  /// \brief The Boolean designer type ID. \return 34048.
  [[nodiscard]] static constexpr ::agiru::Integer Boolean() { return kBoolean; }

  /// \brief The Code designer type ID. \return 31490.
  [[nodiscard]] static constexpr ::agiru::Integer Code() { return kCode; }

  /// \brief The Decimal designer type ID. \return 12800.
  [[nodiscard]] static constexpr ::agiru::Integer Decimal() { return kDecimal; }

  /// \brief The Integer designer type ID. \return 34560.
  [[nodiscard]] static constexpr ::agiru::Integer Integer() { return kInteger; }

  /// \brief The Option designer type ID. \return 35584.
  [[nodiscard]] static constexpr ::agiru::Integer Option() { return kOption; }

  /// \brief The Text designer type ID. \return 31489.
  [[nodiscard]] static constexpr ::agiru::Integer Text() { return kText; }

  /// \brief The Date designer type ID. \return 11776.
  [[nodiscard]] static constexpr ::agiru::Integer Date() { return kDate; }

  /// \brief The Time designer type ID. \return 11777.
  [[nodiscard]] static constexpr ::agiru::Integer Time() { return kTime; }

  /// \brief The DateTime designer type ID. \return 37376.
  [[nodiscard]] static constexpr ::agiru::Integer DateTime() { return kDateTime; }

private:
  static constexpr ::agiru::Integer kBigInteger = 36096;
  static constexpr ::agiru::Integer kBoolean = 34048;
  static constexpr ::agiru::Integer kCode = 31490;
  static constexpr ::agiru::Integer kDecimal = 12801;
  static constexpr ::agiru::Integer kInteger = 34560;
  static constexpr ::agiru::Integer kOption = 35584;
  static constexpr ::agiru::Integer kText = 31489;
  static constexpr ::agiru::Integer kDate = 11776;
  static constexpr ::agiru::Integer kTime = 11777;
  static constexpr ::agiru::Integer kDateTime = 37376;
};

}
