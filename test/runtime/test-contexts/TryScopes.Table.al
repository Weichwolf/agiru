namespace Microsoft.Fixture;

table 50252 "Try Row"
{
    fields
    {
        field(1; Value; Integer) { }
        field(2; Payload; Blob) { }
        field(3; Filter; Integer) { FieldClass = FlowFilter; }
        field(4; "Sort Order"; Integer) { }
    }

    procedure CatchLocal(): Boolean
    begin
        exit(TryFail());
    end;

    procedure SortValue(): Boolean
    begin
        exit(SetCurrentKey("Sort Order"));
    end;

    procedure SortPayload(): Boolean
    begin
        exit(SetCurrentKey(Payload));
    end;

    procedure DiscardPayloadSort()
    begin
        SetCurrentKey(Payload);
    end;

    procedure CatchRecord(): Boolean
    begin
        exit(Rec.TryFail());
    end;

    procedure DiscardLocal()
    begin
        TryFail();
    end;

    procedure SucceedLocal(): Boolean
    begin
        exit(TrySucceed());
    end;

    procedure CatchAssigned(): Boolean
    var
        Result: Boolean;
    begin
        Result := TryFail();
        exit(Result);
    end;

    procedure CatchConditional(): Boolean
    begin
        if TryFail() then
            exit(true);
        exit(false);
    end;

    procedure CatchNegated(): Boolean
    begin
        exit(not TryFail());
    end;

    procedure CatchSelected(): Integer
    begin
        case TryFail() of
            true: exit(1);
            false: exit(2);
        end;
    end;

    [TryFunction]
    procedure TryFail()
    begin
        Value += 1;
        Error('table failure');
    end;

    [TryFunction]
    local procedure TrySucceed()
    begin
        exit;
    end;
}
