namespace Microsoft.Fixture;

codeunit 50196 Probe
{
    procedure VerifyContract(): Boolean
    var
        Value: Text[10];
    begin
        Value := 'before';
        ChooseValue(Value);
        exit(Value = 'after');
    end;

    local procedure ChooseValue(var Value: Integer)
    begin
        Value := 1;
    end;

    local procedure Choosevalue(var Value: Text)
    begin
        Value := 'after';
    end;
}
