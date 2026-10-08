namespace Microsoft.Fixture;

page 50352 "Navigation Modal"
{
    PageType = Card;
    SourceTable = "Navigation Row";
    layout
    {
        area(Content)
        {
            field(ID; Rec.ID) { }
            field(OwnerMarker; OwnerMarker) { }
            field(CloseAttempts; CloseAttempts) { }
            field(ClosedCount; ClosedCount) { }
            field(SeenAction; SeenAction) { }
        }
    }
    trigger OnOpenPage()
    begin
        OwnerMarker += 1;
    end;
    trigger OnQueryClosePage(CloseAction: Action): Boolean
    begin
        CloseAttempts += 1;
        SeenAction := CloseAction;
        if RetryClose then begin
            if CloseAttempts = 1 then
                exit(false);
            if CloseAttempts = 2 then
                Error('Fixture close error');
        end;
        exit(true);
    end;
    trigger OnClosePage()
    begin
        ClosedCount += 1;
    end;
    procedure SetMarker(Value: Integer)
    begin
        OwnerMarker := Value;
    end;
    procedure RequireCloseRetries()
    begin
        RetryClose := true;
    end;
    procedure GetMarker(): Integer
    begin
        exit(OwnerMarker);
    end;
    procedure GetCloseCount(): Integer
    begin
        exit(ClosedCount);
    end;
    var
        OwnerMarker: Integer;
        CloseAttempts: Integer;
        ClosedCount: Integer;
        SeenAction: Action;
        RetryClose: Boolean;
}
