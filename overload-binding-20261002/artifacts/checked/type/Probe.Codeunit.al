namespace Microsoft.Fixture;

codeunit 50196 Probe
{
    procedure VerifyContract(): Boolean
    var
        Value: Text;
    begin
        Value := 'typed';
        exit(Choose(Value) = 2);
    end;

    local procedure Choose(Value: Integer): Integer
    begin
        exit(1);
    end;

    local procedure choose(Value: Text): Integer
    begin
        exit(2);
    end;
}
