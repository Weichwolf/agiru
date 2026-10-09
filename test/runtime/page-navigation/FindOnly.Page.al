namespace Microsoft.Fixture;

page 50353 "Navigation Find Only"
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
    trigger OnFindRecord(Which: Text): Boolean
    begin
        exit(Rec.Find(Which));
    end;
}
