namespace Microsoft.Fixture;

page 50271 "Extension Controls"
{
    PageType = Card;
    layout
    {
        area(Content)
        {
            group(Fields)
            {
                field(Original; Value) { Caption = 'Original caption'; }
            }
        }
    }
    actions
    {
        area(Processing)
        {
            action(OriginalAction) { Caption = 'Original action'; }
        }
    }
    var
        Value: Integer;
}
