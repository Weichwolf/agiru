namespace Microsoft.Fixture;

tableextension 60004 "Owned Row Extension" extends "Owned Row"
{
    fields
    {
        field(2; "Root Extension"; Text[20]) { }
        field(3; "Nested Value"; Text[20])
        {
            MovedFrom = '834a40c9-7a26-46f2-9348-3f6cc8c71719';
        }
    }
}
