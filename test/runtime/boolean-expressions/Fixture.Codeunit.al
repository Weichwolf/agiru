namespace Microsoft.Fixture;

codeunit 50261 "Boolean Expression Consumer"
{
    procedure OrderedAnd(var Trace: Integer): Boolean
    begin
        exit(Push(Trace, 1, false) and Push(Trace, 2, true));
    end;

    procedure OrderedOr(var Trace: Integer): Boolean
    begin
        exit(Push(Trace, 1, true) or Push(Trace, 2, false));
    end;

    procedure OrderedXor(var Trace: Integer): Boolean
    begin
        exit(Push(Trace, 1, true) xor Push(Trace, 2, false));
    end;

    procedure NestedLeft(var Trace: Integer): Boolean
    begin
        exit((Push(Trace, 1, false) and Push(Trace, 2, true)) or Push(Trace, 3, true));
    end;

    procedure NestedRight(var Trace: Integer): Boolean
    begin
        exit(Push(Trace, 1, true) or (Push(Trace, 2, false) and Push(Trace, 3, true)));
    end;

    procedure Negated(var Trace: Integer): Boolean
    begin
        exit(not (Push(Trace, 1, false) and Push(Trace, 2, true)));
    end;

    procedure CaptureBeforeMutation(var Value: Boolean): Boolean
    begin
        exit(Value and MutateFalse(Value));
    end;

    procedure RightAndError(var Trace: Integer): Boolean
    begin
        exit(Push(Trace, 1, false) and Fail(Trace, 2));
    end;

    procedure RightOrError(var Trace: Integer): Boolean
    begin
        exit(Push(Trace, 1, true) or Fail(Trace, 2));
    end;

    procedure LeftError(var Trace: Integer): Boolean
    begin
        exit(Fail(Trace, 1) or Push(Trace, 2, true));
    end;

    procedure CatchAnd(var Trace: Integer): Boolean
    begin
        exit(Push(Trace, 1, false) and TryFail(Trace));
    end;

    procedure CatchOr(var Trace: Integer): Boolean
    begin
        exit(Push(Trace, 1, true) or TryFail(Trace));
    end;

    procedure ChosenBranch(Condition: Boolean; var Trace: Integer): Boolean
    begin
        exit(Condition ? Push(Trace, 1, true) : Push(Trace, 2, false));
    end;

    procedure ConditionContext(var Trace: Integer): Integer
    begin
        if Push(Trace, 1, false) and Push(Trace, 2, true) then
            exit(1);
        exit(2);
    end;

    procedure AssignmentContext(var Trace: Integer): Boolean
    var
        Result: Boolean;
    begin
        Result := Push(Trace, 1, true) or Push(Trace, 2, false);
        exit(Result);
    end;

    local procedure Push(var Trace: Integer; Digit: Integer; Result: Boolean): Boolean
    begin
        Trace := Trace * 10 + Digit;
        exit(Result);
    end;

    local procedure MutateFalse(var Value: Boolean): Boolean
    begin
        Value := false;
        exit(true);
    end;

    local procedure Fail(var Trace: Integer; Digit: Integer): Boolean
    begin
        Trace := Trace * 10 + Digit;
        Error('Boolean producer failed');
    end;

    [TryFunction]
    local procedure TryFail(var Trace: Integer)
    begin
        Trace := Trace * 10 + 2;
        Error('Boolean try failed');
    end;
}
