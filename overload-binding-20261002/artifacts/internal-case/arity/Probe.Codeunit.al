namespace Microsoft.Fixture;

codeunit 50196 Probe
{
    procedure VerifyContract(): Boolean
    begin
        exit(ChooseValue(7) = 7);
    end;

    local procedure ChooseValue(Value: Integer; Extra: Integer): Integer
    begin
        exit(Value + Extra);
    end;

    local procedure Choosevalue(Value: Integer): Integer
    begin
        exit(Value);
    end;
}
