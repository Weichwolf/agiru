namespace Microsoft.Fixture;

table 50197 "Collision Source"
{
    fields
    {
        field(1; "Entry No."; Integer) { }
        field(2; "Record ID"; RecordId) { }
    }
    keys { key(PK; "Entry No.") { Clustered = true; } }
}
