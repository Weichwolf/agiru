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
        }
    }
}
