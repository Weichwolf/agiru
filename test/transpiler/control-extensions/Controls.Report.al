namespace Microsoft.Fixture;

report 50273 "Extension Request Controls"
{
    ProcessingOnly = true;
    dataset { }
    requestpage
    {
        MultipleNewLines = true;
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
    }
    var
        Value: Integer;
}
