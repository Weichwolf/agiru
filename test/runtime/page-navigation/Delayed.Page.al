namespace Microsoft.Fixture;

page 50345 "Navigation Delayed"
{
    PageType = List;
    SourceTable = "Navigation Row";
    DelayedInsert = true;
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
    trigger OnInsertRecord(BelowxRec: Boolean): Boolean
    begin
        if Rec.Value < 0 then
            Error('negative row value');
        exit(true);
    end;
}
