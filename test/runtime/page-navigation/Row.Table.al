namespace Microsoft.Fixture;

table 50340 "Navigation Row"
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
