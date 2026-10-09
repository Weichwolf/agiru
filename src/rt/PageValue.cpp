#include "runtime/PageValue.h"

#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "type/Base64.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/FieldClass.h"
#include "type/Time.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace agiru {
namespace {

std::string_view Tag(FieldType type) {
  switch (type) {
    case FieldType::Text: return "Text";
    case FieldType::Code: return "Code";
    case FieldType::Integer: return "Integer";
    case FieldType::BigInteger: return "BigInteger";
    case FieldType::Decimal: return "Decimal";
    case FieldType::Boolean: return "Boolean";
    case FieldType::Option: return "Option";
    case FieldType::Enum: return "Enum";
    case FieldType::Date: return "Date";
    case FieldType::Time: return "Time";
    case FieldType::DateTime: return "DateTime";
    case FieldType::Duration: return "Duration";
    case FieldType::Guid: return "Guid";
    case FieldType::DateFormula: return "DateFormula";
    case FieldType::RecordId: return "RecordId";
    default: throw Error("Page value type has no scalar transport", "PageValueUnsupported");
  }
}

template <typename T> const T &At(const void *record, const FieldDef &field) {
  return *reinterpret_cast<const T *>(static_cast<const std::byte *>(record) + field.offset);
}

}

namespace {

PageValue ReadScalar(const void *record, const FieldDef &field, std::string_view domain) {
  if (field.fieldClass == FieldClass::FlowFilter) {
    throw Error("A FlowFilter requires its filter-state transport", "PageValueUnsupported");
  }
  PageValue result{.type = std::string(Tag(field.type))};
  switch (field.type) {
    case FieldType::Boolean:
      result.value = static_cast<bool>(At<Boolean>(record, field)) ? "true" : "false";
      break;
    case FieldType::Option:
    case FieldType::Enum: {
      const auto ordinal = At<OrdinalValue>(record, field).AsInteger();
      result.value = std::to_string(ordinal);
      result.domain = domain;
      result.members = field.values;
      result.displayOrdinals = field.displayOrdinals;
      if (const EnumValueDef *member = ValueOf(field.values, ordinal)) {
        result.member = member->name;
      }
      break;
    }
    case FieldType::Date:
      result.value = At<Date>(record, field).ToInvariantString();
      result.undefined = At<Date>(record, field).IsUndefined();
      result.closing = At<Date>(record, field).IsClosing();
      break;
    case FieldType::Time:
      result.value = At<Time>(record, field).ToInvariantString();
      result.undefined = At<Time>(record, field).IsUndefined();
      break;
    case FieldType::DateTime:
      result.value = At<DateTime>(record, field).ToInvariantString();
      result.undefined = At<DateTime>(record, field).IsUndefined();
      break;
    case FieldType::DateFormula:
    case FieldType::Guid: result.value = detail::StorageText(record, field); break;
    case FieldType::RecordId:
      result.value = "base64:" + EncodeBase64(detail::StorageText(record, field));
      break;
    default: result.value = FieldText(record, field); break;
  }
  return result;
}

}

PageValue ReadPageValue(const void *record, const TableDef &table, const FieldDef &field) {
  if (record == nullptr || Field(table, field.no) != &field) {
    throw Error("Page value requires its owning field declaration", "PageValueDeclaration");
  }
  return ReadScalar(record,
                    field,
                    "table/" + std::to_string(table.id.Value()) + "/field/" +
                        std::to_string(field.no.Value()));
}

PageValue ReadPageScalar(const void *value, const FieldDef &declaration, std::string_view domain) {
  if (value == nullptr || declaration.offset != 0 || domain.empty()) {
    throw Error("Page scalar requires typed storage and its declaration identity",
                "PageValueDeclaration");
  }
  return ReadScalar(value, declaration, domain);
}

}
