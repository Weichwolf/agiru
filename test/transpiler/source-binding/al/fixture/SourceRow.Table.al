namespace Microsoft.Fixture;

table 50170 "Source Row"
{
    fields
    {
        field(1; ID; Integer) { }
    }
    procedure Change(var Value: Text[20]; Copy: Text[20])
    begin
        Value := Copy;
        Copy := 'local copy';
    end;
    procedure ChangeOption(var Value: Option Blank,C,D)
    begin
        Value := Value::D;
    end;
    [TryFunction]
    procedure TryChange(var Value: Decimal)
    begin
        Value += 1;
    end;
}
