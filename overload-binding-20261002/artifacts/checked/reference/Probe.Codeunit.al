namespace Microsoft.Fixture;

codeunit 50196 Probe
{
    procedure VerifyContract(): Boolean
    var
        Value: Text[10];
    begin
        Value := 'before';
        Choose(Value);
        exit(Value = 'after');
    end;

    local procedure Choose(var Value: Integer)
    begin
        Value := 1;
    end;

    local procedure choose(var Value: Text)
    begin
        Value := 'after';
    end;
}
