namespace Microsoft.Fixture;

codeunit 50264 "Run Call Forms"
{
    procedure Typed(var Row: Record "Run Row"): Boolean
    var
        Writer: Codeunit "Run Writer";
    begin
        exit(Writer.Run(Row));
    end;

    procedure Static(var Row: Record "Run Row"): Boolean
    begin
        exit(Codeunit.Run(Codeunit::"Run Writer", Row));
    end;

    procedure Dynamic(var Row: Record "Run Row"; Number: Integer): Boolean
    begin
        exit(Codeunit.Run(Number, Row));
    end;

    procedure Statement(var Row: Record "Run Row")
    var
        Writer: Codeunit "Run Writer";
    begin
        Writer.Run(Row);
    end;

    procedure StaticStatement(var Row: Record "Run Row")
    begin
        Codeunit.Run(Codeunit::"Run Writer", Row);
    end;

    procedure DynamicStatement(var Row: Record "Run Row"; Number: Integer)
    begin
        Codeunit.Run(Number, Row);
    end;

    procedure Nested(var Row: Record "Run Row"): Boolean
    var
        NestedRun: Codeunit "Run Nested";
    begin
        exit(NestedRun.Run(Row));
    end;

    procedure CatchArgument(): Boolean
    begin
        exit(TryFail(WriteArgument()));
    end;

    procedure CatchNested(): Boolean
    begin
        exit(TryNested());
    end;

    procedure DiscardArgument()
    begin
        TryFail(WriteArgument());
    end;

    [TryFunction]
    local procedure TryFail(Value: Integer)
    begin
        if Value = 1 then
            Error('after argument');
    end;

    [TryFunction]
    local procedure TryNested()
    begin
        WriteArgument();
        Error('after nested write');
    end;

    local procedure WriteArgument(): Integer
    var
        Stored: Record "Run Buffer";
    begin
        Stored.ID := 91;
        Stored.Value := 1;
        Stored.Insert();
        exit(1);
    end;
}
