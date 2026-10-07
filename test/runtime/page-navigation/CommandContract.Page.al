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
