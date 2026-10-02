namespace Microsoft.Fixture;

codeunit 50196 Probe
{
    procedure VerifyContract(): Boolean
    begin
        exit(Choose('typed') = 'text');
    end;

    local procedure Choose(Value: Guid): Boolean
    begin
        exit(false);
    end;

    local procedure Choose(Value: Text): Text
    begin
        exit('text');
    end;
}
