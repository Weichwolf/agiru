namespace Microsoft.Fixture;

table 50231 "Stored Flag"
{
    DataClassification = SystemMetadata;
    fields
    {
        field(1; ID; Integer) {}
        field(8; Temporary; Boolean) {}
    }
    keys { key(PK; ID) { Clustered = true; } }
}
