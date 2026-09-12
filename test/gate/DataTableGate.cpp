#include "dotnet/BusinessChart.h"
#include "dotnet/CultureInfo.h"
#include "dotnet/DataTable.h"
#include "dotnet/Type.h"
#include "runtime/Error.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/Text.h"
#include "type/Variant.h"

#include "BuiltinsWritten.h"
#include "Check.h"

#include <string>

using agiru::Integer;
using agiru::Variant;
using agiru::dotnet::BusinessChartData;
using agiru::dotnet::CultureInfo;
using agiru::dotnet::DataColumn;
using agiru::dotnet::DataMeasureType;
using agiru::dotnet::DataRow;
using agiru::dotnet::DataTable;
using agiru::dotnet::Type;

namespace {

std::string T(const agiru::Text<0> &text) {
  return std::string(std::string_view(text));
}

/// `System.Type` IS A NAME HERE: `GetType("System.Int32")` carries the full name, `Name` is the
/// part after the last dot, and `Format` renders the full name -- which is what `Business Chart
/// Impl.` reads back out of a column to tell an Integer measure from a Decimal one.
void ATypeIsItsFullName() {
  const Type type = Type::GetType("System.Decimal");
  CHECK_TEXT("FullName", T(type.FullName()), "System.Decimal");
  CHECK_TEXT("Name is the last part", T(type.Name()), "Decimal");
  CHECK_TEXT("Format renders the full name", T(agiru::Format(type)), "System.Decimal");
  CHECK_TRUE("two types of one name are equal", type.Equals(Type::GetType("System.Decimal")));
  CHECK_TRUE("and of another are not", !type.Equals(Type::GetType("System.Int32")));
}

/// A CULTURE IS A TAG AND A NUMBER: `CultureInfo(1033)` is `en-US`, ISO `en`, Windows `ENU`;
/// the invariant culture has no name and LCID 127; a culture's parent is its neutral half; a
/// number the table does not carry keeps the number and says `Unknown` for the names, the way
/// .NET answers a custom culture, rather than refusing.
void ACultureIsATagAndANumber() {
  CultureInfo culture;
  culture = culture.CultureInfo(Integer{1033});
  CHECK_TEXT("en-US by LCID", T(culture.Name()), "en-US");
  CHECK_TRUE("and its LCID", culture.LCID() == 1033);
  CHECK_TEXT("two-letter ISO", T(culture.TwoLetterISOLanguageName()), "en");
  CHECK_TEXT("three-letter Windows", T(culture.ThreeLetterWindowsLanguageName()), "ENU");
  CHECK_TEXT("the parent is the neutral culture", T(culture.Parent().Name()), "en");
  culture = culture.CultureInfo("de-de");
  CHECK_TRUE("by name, without regard to case", culture.LCID() == 1031);
  CHECK_TEXT("the invariant culture has no name", T(CultureInfo::InvariantCulture().Name()), "");
  CHECK_TRUE("and LCID 127", CultureInfo::InvariantCulture().LCID() == 127);
  CHECK_TEXT("an unknown number keeps its number and says Unknown",
             T(CultureInfo::GetCultureInfo(Integer{9999}).ThreeLetterWindowsLanguageName()),
             "Unknown");
  CHECK_TRUE("and its LCID", CultureInfo::GetCultureInfo(Integer{9999}).LCID() == 9999);
}

/// A `DataTable` IS COLUMNS AND ROWS OF VARIANTS, AND EVERY PIECE IS A REFERENCE: a column added
/// to the collection and then captioned is captioned in the table; a row `NewRow` made, written
/// through `Item(name, value)` and added is the row the table holds; the values come back typed.
void ATableIsColumnsAndRowsAndEveryPieceIsAReference() {
  DataTable table;
  table = table.DataTable("DataTable");
  CHECK_TEXT("the name", T(table.TableName()), "DataTable");
  DataColumn column;
  column = column.DataColumn("Month");
  column.DataType(Type::GetType("System.String"));
  table.Columns().Add(column);
  table.Columns().Add("Amount").DataType(Type::GetType("System.Decimal"));
  CHECK_TRUE("two columns", table.Columns().Count() == 2);
  CHECK_TRUE("Contains finds one without regard to case", table.Columns().Contains("amount"));
  CHECK_TRUE("and not another", !table.Columns().Contains("Total"));
  table.Columns().Item(Integer{0}).Caption("Period");
  CHECK_TEXT("a caption set through the collection is the table's",
             T(column.Caption()),
             "Period");
  CHECK_TEXT("the type reads back through Format",
             T(agiru::Format(table.Columns().Item("Amount").DataType())),
             "System.Decimal");
  DataRow row = table.NewRow();
  row.Item("Month", Variant(std::string("Jan")));
  row.Item("Amount", Variant(agiru::Decimal::FromInvariantString("12.5")));
  table.Rows().Add(row);
  DataRow second = table.NewRow();
  second.Item(Integer{0}, Variant(std::string("Feb")));
  table.Rows().Add(second);
  CHECK_TRUE("two rows", table.Rows().Count() == 2);
  CHECK_TEXT("a value written through the row is in the table",
             T(agiru::Format(table.Rows().Item(Integer{0}).Item("Amount"))),
             "12.5");
  CHECK_TEXT("by position too", T(agiru::Format(table.Rows().Item(Integer{1}).Item("Month"))),
             "Feb");
  CHECK_TRUE("an unset value is an empty Variant",
             table.Rows().Item(Integer{1}).Item("Amount").IsEmpty());
  std::string refusal;
  try {
    static_cast<void>(row.Item("Total"));
  } catch (const agiru::Error &e) { refusal = e.what(); }
  CHECK_TEXT("a column the table lacks is refused by name",
             refusal,
             "DataRow: there is no column 'Total'");
  table.Clear();
  CHECK_TRUE("Clear drops the rows", table.Rows().Count() == 0);
  CHECK_TRUE("and keeps the columns", table.Columns().Count() == 2);
  table.Locale(CultureInfo::InvariantCulture());
  CHECK_TRUE("Locale holds the culture", table.Locale().LCID() == 127);
}

/// `BusinessChartData` IS THE CHART'S SHAPE: the X dimension, the measures with how each is
/// drawn -- a `DataMeasureType` assigned from the ordinal of `Business Chart Type` -- and the
/// table. `ClearMeasures` forgets the measures and keeps the rest.
void ChartDataHoldsTheShape() {
  BusinessChartData data;
  data = data.BusinessChartData();
  data.XDimension("Month");
  DataMeasureType type;
  type = Integer{5};
  data.AddMeasure("Amount", type);
  data.AddMeasure("Count", DataMeasureType::Line());
  CHECK_TEXT("the X dimension", T(data.XDimension()), "Month");
  CHECK_TRUE("two measures", data.Measures().size() == 2);
  CHECK_TRUE("the first drawn as the ordinal said", data.Measures()[0].type.AsInteger() == 5);
  CHECK_TRUE("the second as a line", data.Measures()[1].type.AsInteger() == 2);
  data.ShowChartCondensed(true);
  CHECK_TRUE("condensed", static_cast<bool>(data.ShowChartCondensed()));
  data.ClearMeasures();
  CHECK_TRUE("ClearMeasures forgets them", data.Measures().empty());
  CHECK_TEXT("and keeps the dimension", T(data.XDimension()), "Month");
}

}

int main() {
  return gate::Run("DataTable", [] {
    ATypeIsItsFullName();
    ACultureIsATagAndANumber();
    ATableIsColumnsAndRowsAndEveryPieceIsAReference();
    ChartDataHoldsTheShape();
  });
}
