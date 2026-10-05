namespace Microsoft.Fixture;

tableextension 60002 "Implicit Row Extension" extends "Implicit Row"
{
    AllowInCustomizations = AsReadWrite;
    fields
    {
        field(1; "Earlier Extension"; Text[20]) { }
        field(2; "Private Extension"; Text[20]) { AllowInCustomizations = Never; }
    }
    keys
    {
        key(ExtensionKey; "Earlier Extension") { }
    }
}
