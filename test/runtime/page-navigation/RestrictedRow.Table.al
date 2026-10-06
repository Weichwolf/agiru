namespace Microsoft.Fixture;

table 50348 "Restricted Row"
{
    fields
    {
        field(1; ID; Integer) { }
        field(2; Value; Integer) { }
    }
    keys
    {
        key(PK; ID) { Clustered = true; }
    }
}
