namespace Microsoft.Fixture;

codeunit 50172 Caller
{
    procedure Run(): Boolean
    var
        Row: Record "Source Row" temporary;
        Value: Text[30];
        Copy: Text[30];
        State: Option Blank,A,B;
        Amount: Decimal;
    begin
        Value := 'original';
        Copy := 'updated';
        Row.Change(Value, Copy);
        if Value <> 'updated' then
            Error('Reference argument was not written back');
        if Copy <> 'updated' then
            Error('Value argument escaped its copy');
        Row.AddedChange(Value);
        if Value <> 'extension' then
            Error('Extension reference argument was not written back');
        State := State::A;
        Row.ChangeOption(State);
        if State <> State::B then
            Error('Option reference argument was not written back');
        State := State::A;
        OwnChange(State);
        if State <> State::B then
            Error('Own Option reference argument was not written back');
        if not Row.TryChange(Amount) then
            Error('TryFunction call failed');
        if Amount <> 1 then
            Error('Decimal reference argument was not written back');
        exit(true);
    end;
    local procedure OwnChange(var Value: Option Blank,C,D)
    begin
        Value := Value::D;
    end;
    procedure ReadIdentity(var Row: Record "Source Row"; Id: Guid): Boolean
    begin
        exit(Row.GetBySystemId(Id));
    end;
    procedure RequireIdentity(var Row: Record "Source Row"; Id: Guid)
    begin
        Row.GetBySystemId(Id);
    end;
    procedure ReadRefIdentity(var Row: RecordRef; Id: Guid): Boolean
    begin
        exit(Row.GetBySystemId(Id));
    end;
    procedure RequireRefIdentity(var Row: RecordRef; Id: Guid)
    begin
        Row.GetBySystemId(Id);
    end;
}
