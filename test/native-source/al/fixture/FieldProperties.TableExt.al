namespace System.Reflection;

tableextension 50173 "Field Properties Extension" extends "Field Properties Fixture"
{
    AllowInCustomizations = Never;

    fields
    {
        field(50173; "Extension Default"; Integer) { }
        field(50174; "Extension Override"; Integer)
        {
            AllowInCustomizations = AsReadOnly;
            DataClassification = EndUserIdentifiableInformation;
        }
        modify("Modified Field")
        {
            Caption = 'Changed caption';
        }
        modify("Conditional Reference")
        {
            TableRelation = if (ID = const(2)) "Field Properties Fixture".ID;
        }
        modify("Fixed Reference")
        {
            TableRelation = "Field Defaults Fixture".ID;
        }
        modify("Fallback Reference")
        {
            TableRelation = if (ID = const(2)) "Field Defaults Fixture".ID;
        }
    }
}
