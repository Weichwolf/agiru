namespace Microsoft.Fixture;

tableextension 60002 "Implicit Row Extension" extends "Implicit Row"
{
    fields
    {
        field(1; "Earlier Extension"; Text[20]) { }
    }
    keys
    {
        key(ExtensionKey; "Earlier Extension") { }
    }
}
