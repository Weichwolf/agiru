namespace System.Reflection;

table 50172 "Field Properties Fixture"
{
    DataClassification = CustomerContent;
    AllowInCustomizations = AsReadWrite;
    Access = Internal;

    fields
    {
        field(1; ID; Integer) { }
        field(2; "Numeric Code"; Code[20])
        {
            DataClassification = AccountData;
            SqlDataType = Integer;
            Access = Protected;
            AllowInCustomizations = Never;
        }
        field(3; Description; Text[100])
        {
            OptimizeForTextSearch = true;
        }
        field(4; "Flow Filter"; Code[20])
        {
            FieldClass = FlowFilter;
        }
        field(5; "Modified Field"; Integer)
        {
            DataClassification = ToBeClassified;
            Caption = 'Original caption';
        }
        field(6; "Conditional Reference"; Integer)
        {
            TableRelation = if (ID = const(1)) "Field Defaults Fixture".ID;
        }
        field(7; "Fixed Reference"; Integer)
        {
            TableRelation = "Field Properties Fixture".ID;
        }
        field(8; "Fallback Reference"; Integer)
        {
            TableRelation = if (ID = const(1)) "Field Defaults Fixture".ID
                            else "Field Properties Fixture".ID;
        }
    }

    keys
    {
        key(PK; ID) { Clustered = true; }
    }
}
