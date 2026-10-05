namespace Microsoft.Fixture;

table 60001 "Implicit Row"
{
    Caption = 'Different table caption';
    Scope = Cloud;
    ObsoleteState = Pending;
    ObsoleteReason = 'Source-owned reflection fixture';
    DataClassification = AccountData;
    AllowInCustomizations = Never;
    CompressionType = Row;
    Access = Internal;
    DataCaptionFields = Later, "Primary ID";
    DataPerCompany = false;
    ReplicateData = false;
    PasteIsValid = false;
    LookupPageId = 50176;
    DrillDownPageId = 50177;

    fields
    {
        field(30; Later; Text[40]) { }
        field(10; "Primary ID"; Integer) { AllowInCustomizations = AsReadOnly; }
    }
}
