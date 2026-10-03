namespace Microsoft.Fixture;

tableextension 50171 "Source Row Extension" extends "Source Row"
{
    fields
    {
        field(2; "Added Flag"; Boolean) { }
        field(3; "Chan-ge"; Boolean) { }
    }
    procedure AddedChange(var Value: Text[20])
    begin
        Value := 'extension';
    end;
}
