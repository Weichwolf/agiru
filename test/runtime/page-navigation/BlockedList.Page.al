namespace Microsoft.Fixture;

page 50344 "Navigation Blocked List"
{
    PageType = List;
    SourceTable = "Navigation Row";
    CardPageId = "Navigation Blocked Card";
    layout
    {
        area(Content)
        {
            repeater(Rows)
            {
                field(ID; Rec.ID) { }
            }
        }
    }
}
