namespace Microsoft.Fixture;

codeunit 50260 "Text Position Consumer"
{
    procedure ReadPosition(Value: Text; Position: Integer): Integer
    begin
        exit(Value[Position]);
    end;

    procedure ReplacePosition(Value: Text; Position: Integer; Character: Char): Text
    begin
        Value[Position] := Character;
        exit(Value);
    end;

    procedure CopyPosition(Value: Text; Source: Text; Position: Integer; SourcePosition: Integer): Text
    begin
        Value[Position] := Source[SourcePosition];
        exit(Value);
    end;

    procedure ReplaceEach(Value: Text; Character: Char): Text
    var
        Input: Text;
        Position: Integer;
    begin
        Input := Value;
        for Position := 1 to StrLen(Input) do
            Value[Position] := Character;
        exit(Value);
    end;

    procedure AppendBounded(Value: Text): Text
    var
        Bounded: Text[3];
    begin
        Bounded := Value;
        Bounded[StrLen(Bounded) + 1] := 8364;
        exit(Bounded);
    end;
}
