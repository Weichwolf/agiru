#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief The AL system table `Company` (2000000006) -- which companies the tenant holds.

namespace agiru::platform {

/// \brief AL `Company` -- the tenant's register of companies, which no `.al` file in BCApps
///        declares.
///
/// \note THE DECLARATION IS THE SYSTEM SYMBOLS', not a measurement.
///       `work/symbols/src/Tenant Database Tables/Company.Table.al` (`make symbols`) carries it
///       field for field, and it is the reason the numbers are not 1 to 5: `Id` and
///       `Business Profile Id` were added in the 8000 range, which a column order cannot see
///       (board:0607).
///
/// \note `DataPerCompany = false` AND `ReplicateData = false`. It describes the companies, so it
///       cannot live inside one.
class Company_Table : public Table<Company_Table> {
public:
  /// \brief The AL table number.
  static constexpr TableId kId{2000000006};

  /// \brief The AL name.
  static constexpr std::string_view kName{"Company"};

  detail::StateHandle State_Block;

  /// \brief The declared length of `Name`, which is AL's and not this file's.
  static constexpr std::size_t kNameLength = 30;
  /// \brief The declared length of `Display Name` and `Business Profile Id`.
  static constexpr std::size_t kDisplayNameLength = 250;

  /// \brief AL `Company.Name` -- the identifier the database partitions on.
  Text<kNameLength> Name;
  /// \brief AL `Company."Evaluation Company"`.
  Boolean EvaluationCompany{};
  /// \brief AL `Company."Display Name"`.
  Text<kDisplayNameLength> DisplayName;
  /// \brief AL `Company.Id`.
  Guid Id;
  /// \brief AL `Company."Business Profile Id"`.
  Text<kDisplayNameLength> BusinessProfileId;

  /// \brief AL `Company.SystemId`.
  Guid SystemId;
  /// \brief AL `Company.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `Company.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `Company.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `Company.SystemModifiedBy`.
  Guid SystemModifiedBy;

  /// \brief The field numbers, from the system symbols' declaration.
  struct Field_No {
    /// \brief The AL field number of `Name`.
    static constexpr ::agiru::FieldNo Name{1};
    /// \brief The AL field number of `Evaluation Company`.
    static constexpr ::agiru::FieldNo EvaluationCompany{2};
    /// \brief The AL field number of `Display Name`.
    static constexpr ::agiru::FieldNo DisplayName{3};
    /// \brief The AL field number of `Id`, which is in the 8000 range and not the fourth.
    static constexpr ::agiru::FieldNo Id{8000};
    /// \brief The AL field number of `Business Profile Id`.
    static constexpr ::agiru::FieldNo BusinessProfileId{8005};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::Name}};
};

/// \brief AL `Company`, under the name AL gives it.
using Company = Company_Table;

/// \brief The field table of the system `Company` table.
inline constexpr auto kCompanyFields = WithSystemFields<Company>(std::array<FieldDef, 5>{{
    Declare<&Company::Name>(Company::Field_No::Name, "Name", "Name", offsetof(Company, Name)),
    Declare<&Company::EvaluationCompany>(Company::Field_No::EvaluationCompany,
                                         "Evaluation Company",
                                         "Evaluation Company",
                                         offsetof(Company, EvaluationCompany)),
    Declare<&Company::DisplayName>(Company::Field_No::DisplayName,
                                   "Display Name",
                                   "Display Name",
                                   offsetof(Company, DisplayName)),
    Declare<&Company::Id>(Company::Field_No::Id, "Id", "Id", offsetof(Company, Id)),
    Declare<&Company::BusinessProfileId>(Company::Field_No::BusinessProfileId,
                                         "Business Profile Id",
                                         "Business Profile Id",
                                         offsetof(Company, BusinessProfileId)),
}});

/// \brief The keys of the system `Company` table.
inline constexpr std::array<KeyDef, 1> kCompanyKeys{{
    KeyDef{.name = "Key1", .fields = Company::kKey1, .clustered = true},
}};

/// \brief The declaration of the system `Company` table.
inline constexpr TableDef kCompanyTable{
    .id = Company::kId,
    .name = Company::kName,
    .caption = Company::kName,
    .fields = kCompanyFields,
    .keys = kCompanyKeys,
};

static_assert(FieldsAreSorted(kCompanyTable), "the field table is searched by number");

}

/// \brief What the runtime reaches the system `Company` table through.
template <> struct agiru::TableTraits<agiru::platform::Company> {
  /// \brief The table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kCompanyTable;
};
