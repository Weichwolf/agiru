namespace Microsoft.Fixture;

page 50342 "Navigation Override"
{
    PageType = List;
    SourceTable = "Navigation Row";
    CardPageId = "Navigation Card";
    layout
    {
        area(Content)
        {
            repeater(Rows)
            {
                field(ID; Rec.ID) { }
                field(Value; Rec.Value) { }
            }
        }
    }
    actions
    {
        area(Processing)
        {
            action(Edit)
            {
                trigger OnAction()
                begin
                    Rec.Value += 100;
                    Rec.Modify();
                end;
            }
        }
    }
}
