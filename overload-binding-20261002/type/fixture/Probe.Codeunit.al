namespace Microsoft.Fixture;

codeunit 50196 Probe
{
    procedure VerifyContract(): Boolean
    var
        Value: Text;
    begin
        Value := 'typed';
        exit(ChooseValue(Value) = 2);
    end;

    local procedure ChooseValue(Value: Integer): Integer
    begin
        exit(1);
    end;

    local procedure Choosevalue(Value: Text): Integer
    begin
        exit(2);
    end;
}
