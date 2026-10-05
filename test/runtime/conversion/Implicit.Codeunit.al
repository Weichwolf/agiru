namespace Microsoft.Fixture;

codeunit 50266 "Implicit Byte Array Consumer"
{
    procedure DecodeLength(Input: Text): Integer
    var
        Convert: DotNet Convert;
    begin
        exit(Convert.FromBase64String(Input).Length());
    end;
}
