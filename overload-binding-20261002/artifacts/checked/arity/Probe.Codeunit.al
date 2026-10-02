namespace Microsoft.Fixture;

codeunit 50196 Probe
{
    procedure VerifyContract(): Boolean
    begin
        exit(Choose(7) = 7);
    end;

    local procedure Choose(Value: Integer; Extra: Integer): Integer
    begin
        exit(Value + Extra);
    end;

    local procedure choose(Value: Integer): Integer
    begin
        exit(Value);
    end;
}
