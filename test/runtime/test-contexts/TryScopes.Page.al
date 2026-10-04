namespace Microsoft.Fixture;

page 50253 "Try Page"
{
    PageType = Card;
    SourceTable = "Try Row";

    procedure CatchLocal(): Boolean
    begin
        exit(TryFail());
    end;

    procedure CatchRecord(): Boolean
    begin
        exit(Rec.TryFail());
    end;

    procedure DiscardLocal()
    begin
        TryFail();
    end;

    [TryFunction]
    local procedure TryFail()
    begin
        Error('page failure');
    end;
}
