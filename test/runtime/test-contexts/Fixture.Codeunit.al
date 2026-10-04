namespace Microsoft.Fixture;

codeunit 50250 "Test Context Consumer"
{
    procedure SelectValue(var Row: Record "Try Row"): Boolean
    begin
        exit(Row.SetCurrentKey("Sort Order"));
    end;

    procedure SelectFilter(var Row: Record "Try Row"): Boolean
    begin
        exit(Row.SetCurrentKey(Filter));
    end;

    procedure DiscardFilterSort(var Row: Record "Try Row")
    begin
        Row.SetCurrentKey(Filter);
    end;

    procedure CatchSelf(): Boolean
    begin
        exit(this.TryFail());
    end;

    procedure DiscardSelf()
    begin
        this.TryFail();
    end;

    procedure DiscardOuter(var Result: Boolean)
    begin
        Store(this.TryFail(), Result);
    end;

    procedure CatchOuter(var Result: Boolean): Boolean
    begin
        exit(TryStore(this.TryFail(), Result));
    end;

    procedure CatchArgumentError(var Result: Boolean): Boolean
    begin
        exit(TryStore(OrdinaryFail(), Result));
    end;

    local procedure Store(Value: Boolean; var Result: Boolean)
    begin
        Result := Value;
    end;

    [TryFunction]
    local procedure TryStore(Value: Boolean; var Result: Boolean)
    begin
        Store(Value, Result);
    end;

    local procedure OrdinaryFail(): Boolean
    begin
        Error('argument failure');
    end;

    procedure SelectOnce(var Count: Integer): Integer
    begin
        case NextCount(Count) of
            1: exit(1);
            2 .. 3: exit(2);
            else exit(0);
        end;
    end;

    local procedure NextCount(var Count: Integer): Integer
    begin
        Count += 1;
        exit(Count);
    end;

    [TryFunction]
    local procedure TryFail()
    begin
        Error('codeunit failure');
    end;

    procedure Provider(Context: DataSourceContext): Integer
    begin
        exit(Context.CodeunitId);
    end;

    procedure TestApp(Context: DataSourceContext): Guid
    begin
        exit(Context.AppId());
    end;

    procedure Identity(Context: TestHandlerContext): Text
    begin
        exit(StrSubstNo('%1|%2|%3|%4|%5', Context.CodeunitId,
            Context.CodeunitName(), Context.ProcedureName, Context.TestCaseName(), Context.Success));
    end;

    procedure CopySkip(Context: TestHandlerContext)
    var
        Copy: TestHandlerContext;
    begin
        Copy := Context;
        Copy.Skip('requested by AL copy');
    end;
}
