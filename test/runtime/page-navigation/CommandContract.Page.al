namespace Microsoft.Fixture;

page 50347 "Command Contract Card"
{
    PageType = Card;
    SourceTable = "Navigation Row";
    layout
    {
        area(Content)
        {
            field(ID; Rec.ID) { }
            field(Value; Rec.Value) { }
        }
    }
    actions
    {
        area(Processing)
        {
            action(ReadOtherTable)
            {
                trigger OnAction()
                var
                    Restricted: Record "Restricted Row";
                begin
                    Restricted.Get(1);
                    Rec.Value := Restricted.Value;
                    Rec.Modify();
                end;
            }
            action(WriteOtherTable)
            {
                trigger OnAction()
                var
                    Restricted: Record "Restricted Row";
                begin
                    Restricted.ID := 2;
                    Restricted.Value := 999;
                    Restricted.Insert();
                end;
            }
            action(WriteAndFail)
            {
                trigger OnAction()
                begin
                    Rec.Value += 100;
                    Rec.Modify();
                    Error('rollback fixture error');
                end;
            }
            action(CommitAndFail)
            {
                trigger OnAction()
                begin
                    Rec.Value += 100;
                    Rec.Modify();
                    Commit();
                    Error('durable fixture error');
                end;
            }
            action(CaughtTryWrite)
            {
                trigger OnAction()
                begin
                    if AttemptWrite() then
                        Error('try fixture must fail');
                end;
            }
            action(ConfirmWrite)
            {
                trigger OnAction()
                begin
                    if not GuiAllowed() then
                        Error('interactive transport missing');
                    Message('Before question <script> 東京.');
                    Rec.Value += 10;
                    Rec.Modify();
                    if not Confirm('Save value %1?', true, Rec.Value) then
                        Error('explicit decline');
                    Message('Saved value %1.', Rec.Value);
                end;
            }
            action(MenuWrite)
            {
                trigger OnAction()
                var
                    Selected: Integer;
                begin
                    Selected := Dialog.StrMenu('First,Second,東京', 2, 'Choose explicitly <script>.');
                    Rec.Value := Selected;
                    Rec.Modify();
                    Message('Selected %1.', Selected);
                end;
            }
            action(TwoQuestions)
            {
                trigger OnAction()
                begin
                    if Confirm('First?', false) then
                        if Confirm('Second?', true) then begin
                            Rec.Value += 1;
                            Rec.Modify();
                        end;
                end;
            }
            action(CommittedConfirm)
            {
                trigger OnAction()
                begin
                    Rec.Value += 10;
                    Rec.Modify();
                    Commit();
                    if not Confirm('Continue after commit?', true) then
                        Error('declined after commit');
                end;
            }
            action(OversizedQuestion)
            {
                trigger OnAction()
                begin
                    Rec.Value += 10;
                    Rec.Modify();
                    if Confirm(PadStr('', 300000, '&'), true) then
                        Message('unreachable oversized question');
                end;
            }
        }
    }

    [TryFunction]
    local procedure AttemptWrite()
    var
        Row: Record "Navigation Row";
    begin
        Row.Get(1);
        Row.Value := 777;
        Row.Modify();
        Error('caught try fixture error');
    end;
}
