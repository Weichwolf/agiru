namespace Microsoft.Fixture;

table 60001 "Implicit Row"
{
    Caption = 'Different table caption';
    Scope = Cloud;
    ObsoleteState = Pending;
    ObsoleteReason = 'Source-owned reflection fixture';
    DataClassification = AccountData;

    fields
    {
        field(30; Later; Text[40]) { }
        field(10; "Primary ID"; Integer) { }
    }
}
