namespace Microsoft.Fixture;

table 50198 ProfileRow
{
    TableType = Normal;
    LinkedObject = false;
    fields
    {
        field(1; ID; Integer) { }
        field(2; Version; BigInteger) { SqlTimestamp = true; }
    }
    keys { key(PK; ID) { } }
}
