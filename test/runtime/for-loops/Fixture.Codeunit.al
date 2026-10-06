namespace Microsoft.Fixture;

codeunit 50262 "For Loop Consumer"
{
    var
        GlobalCounter: Integer;
        Queue: List of [Variant];

    procedure QueuedValues(): Text
    var
        I: Integer;
        Result: Text;
    begin
        Clear(Queue);
        Queue.Add(2);
        Queue.Add('LOT1');
        Queue.Add('LOT2');
        for I := 1 to DequeueInteger() do
            Result += DequeueText();
        exit(Result);
    end;

    procedure RemainingQueue(): Integer
    begin
        exit(Queue.Count());
    end;

    procedure OrderedBounds(var Trace: Integer; var Calls: Integer; Descending: Boolean): Integer
    var
        I: Integer;
        Total: Integer;
    begin
        if Descending then
            for I := IntegerBound(Trace, Calls, 3) downto IntegerBound(Trace, Calls, 1) do
                Total += I
        else
            for I := IntegerBound(Trace, Calls, 1) to IntegerBound(Trace, Calls, 3) do
                Total += I;
        exit(Total);
    end;

    procedure MutableBound(Descending: Boolean): Integer
    var
        I: Integer;
        Bound: Integer;
        Total: Integer;
    begin
        if Descending then begin
            Bound := 1;
            for I := 3 downto Bound do begin
                Total += I;
                Bound := 3;
            end;
        end else begin
            Bound := 3;
            for I := 1 to Bound do begin
                Total += I;
                Bound := 1;
            end;
        end;
        exit(Total);
    end;

    procedure EmptyBounds(var Trace: Integer; var Calls: Integer; Descending: Boolean): Integer
    var
        I: Integer;
        Count: Integer;
    begin
        if Descending then
            for I := IntegerBound(Trace, Calls, 1) downto IntegerBound(Trace, Calls, 3) do
                Count += 1
        else
            for I := IntegerBound(Trace, Calls, 3) to IntegerBound(Trace, Calls, 1) do
                Count += 1;
        exit(Count);
    end;

    procedure BooleanBounds(var Calls: Integer; Descending: Boolean): Integer
    var
        Flag: Boolean;
        Bound: Boolean;
        Total: Integer;
    begin
        Bound := not Descending;
        if Descending then
            for Flag := true downto BooleanBound(Calls, Bound) do begin
                Total := Total * 10 + (Flag ? 2 : 1);
                Bound := true;
            end
        else
            for Flag := false to BooleanBound(Calls, Bound) do begin
                Total := Total * 10 + (Flag ? 2 : 1);
                Bound := false;
            end;
        exit(Total);
    end;

    procedure GlobalAndNested(): Integer
    var
        J: Integer;
        Total: Integer;
    begin
        for GlobalCounter := 1 to 2 do
            for J := 1 to GlobalCounter do
                Total += GlobalCounter * 10 + J;
        exit(Total);
    end;

    procedure VarCounter(var Counter: Integer): Integer
    var
        Total: Integer;
    begin
        for Counter := 1 to 3 do
            Total += Counter;
        exit(Total);
    end;

    procedure ControlTransfers(): Integer
    var
        I: Integer;
        Total: Integer;
    begin
        for I := 1 to 5 do begin
            if I = 2 then
                continue;
            if I = 4 then
                break;
            Total += I;
        end;
        for I := 7 downto 1 do begin
            if I = 6 then
                continue;
            if I = 3 then
                break;
            Total += I;
        end;
        exit(Total);
    end;

    procedure ExactBigInteger(): BigInteger
    var
        I: BigInteger;
        Total: BigInteger;
    begin
        for I := 9007199254740993L to 9007199254740994L do
            Total += I;
        exit(Total);
    end;

    procedure TemporaryNames(): Integer
    var
        I: Integer;
        ForEnd_Block_1: Integer;
        agiruForEnd_Block_1: Integer;
        Step_Block_2: Boolean;
        Total: Integer;
    begin
        ForEnd_Block_1 := 3;
        agiruForEnd_Block_1 := 4;
        for I := 1 to ForEnd_Block_1 do
            Total += I;
        for Step_Block_2 := false to true do
            Total += Step_Block_2 ? agiruForEnd_Block_1 : 1;
        exit(Total);
    end;

    procedure OptionBounds(): Integer
    var
        Choice: Option Low,Middle,High;
        Bound: Option Low,Middle,High;
        Total: Integer;
    begin
        Bound := Bound::High;
        for Choice := Choice::Low to Bound do begin
            Total := Total * 10 + Choice + 1;
            Bound := Bound::Low;
        end;
        exit(Total);
    end;

    procedure FailedBounds(var Trace: Integer; var Calls: Integer; FailFirst: Boolean)
    var
        I: Integer;
    begin
        if FailFirst then
            for I := FailBound(Trace, Calls) to IntegerBound(Trace, Calls, 3) do
                Trace := 99
        else
            for I := IntegerBound(Trace, Calls, 1) to FailBound(Trace, Calls) do
                Trace := 99;
    end;

    local procedure IntegerBound(var Trace: Integer; var Calls: Integer; Value: Integer): Integer
    begin
        Calls += 1;
        Trace := Trace * 10 + Value;
        exit(Value);
    end;

    local procedure DequeueInteger(): Integer
    var
        Value: Variant;
    begin
        Queue.Get(1, Value);
        Queue.RemoveAt(1);
        exit(Value);
    end;

    local procedure DequeueText(): Text
    var
        Value: Variant;
    begin
        Queue.Get(1, Value);
        Queue.RemoveAt(1);
        exit(Value);
    end;

    local procedure BooleanBound(var Calls: Integer; Value: Boolean): Boolean
    begin
        Calls += 1;
        exit(Value);
    end;

    local procedure FailBound(var Trace: Integer; var Calls: Integer): Integer
    begin
        Calls += 1;
        Trace := Trace * 10 + 7;
        Error('Loop bound failed');
    end;
}
