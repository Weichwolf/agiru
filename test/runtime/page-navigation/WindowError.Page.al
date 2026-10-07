namespace Microsoft.Fixture;

page 50351 "Navigation Window Error"
{
    PageType = List;
    Editable = false;
    SourceTable = "Navigation Row";
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
    trigger OnAfterGetRecord()
    begin
        if Rec.ID = 2 then
            Error('Window trigger error');
        Rec.Value := 99;
        Rec.Modify();
    end;
}
