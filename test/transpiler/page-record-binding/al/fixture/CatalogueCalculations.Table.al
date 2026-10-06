namespace Microsoft.Fixture;

table 50200 "Catalogue Calculations"
{
    Caption = 'Independent calculation source';
    fields
    {
        field(1; "Table ID"; Integer) { }
        field(2; "Field ID"; Integer) { Caption = 'Field identity'; }
        field(3; "Page ID"; Integer) { }
        field(4; "Table Caption"; Text[250])
        {
            FieldClass = FlowField;
            CalcFormula = lookup("Table Metadata".Caption where(ID = field("Table ID")));
        }
        field(5; "Field Caption"; Text[80])
        {
            FieldClass = FlowField;
            CalcFormula = lookup(Field."Field Caption" where(TableNo = field("Table ID"), "No." = field("Field ID")));
        }
        field(6; "Page Caption"; Text[250])
        {
            FieldClass = FlowField;
            CalcFormula = lookup("Page Metadata".Caption where(ID = field("Page ID")));
        }
    }
    keys { key(Primary; "Table ID", "Field ID", "Page ID") { } }
}
