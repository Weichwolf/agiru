namespace Microsoft.Fixture;

page 50341 "Navigation List"
{
    PageType = List;
    Editable = false;
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
                field(Label; Rec.Label) { }
                field(Amount; Rec.Amount) { }
                field(Exact; Rec.Exact) { }
                field(Code; Rec.Code) { }
            }
        }
    }
}
