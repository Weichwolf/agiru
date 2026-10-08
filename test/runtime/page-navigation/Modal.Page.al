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
            field(OwnerMarker; OwnerMarker)
            {
                trigger OnValidate()
                begin
                    ValidationCount += 1;
                end;
            }
            field(ValidationCount; ValidationCount) { Editable = false; }
            field(CloseAttempts; CloseAttempts) { }
            field(ClosedCount; ClosedCount) { }
            field(SeenAction; SeenAction) { }
            field(ExactAmount; ExactAmount) { }
            field(ExactInteger; ExactInteger) { }
            field(OriginalText; OriginalText) { }
            field(ArrayValue; ArrayValues[2]) { }
            field(Choice; Choice) { }
        }
    }
    actions
    {
        area(Processing)
        {
            action(Notify)
            {
                trigger OnAction()
                begin
                    Message('Modal <script> Grün');
                end;
            }
            action(PickChild)
            {
                trigger OnAction()
                var
                    Lookup: Page "Navigation List";
                    Selected: Record "Navigation Row";
                begin
                    Selected.SetRange(ID, 2);
                    Lookup.SetTableView(Selected);
                    Lookup.LookupMode(true);
                    if Lookup.RunModal() = Action::LookupOK then
                        OwnerMarker += 10;
                end;
            }
            action(Ask)
            {
                trigger OnAction()
                begin
                    if Confirm('Keep the original modal?', false) then
                        OwnerMarker += 10;
                end;
            }
        }
    }
    trigger OnOpenPage()
    begin
        OwnerMarker += 1;
        Evaluate(ExactAmount, '1.2300');
        Evaluate(ExactInteger, '9223372036854775807');
        OriginalText := 'Grüezi <script> & "quoted"';
        ArrayValues[2] := 7;
        Choice := Choice::After;
    end;
    trigger OnQueryClosePage(CloseAction: Action): Boolean
    var
        CloseRow: Record "Navigation Row";
    begin
        CloseAttempts += 1;
        SeenAction := CloseAction;
        if RetryClose then begin
            CloseRow.Get(2);
            CloseRow.Modify();
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
        ValidationCount: Integer;
        CloseAttempts: Integer;
        ClosedCount: Integer;
        SeenAction: Action;
        RetryClose: Boolean;
        ExactAmount: Decimal;
        ExactInteger: BigInteger;
        OriginalText: Text;
        ArrayValues: array[2] of Integer;
        Choice: Option Before,After;
}
