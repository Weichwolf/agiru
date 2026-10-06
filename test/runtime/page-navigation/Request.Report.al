namespace Microsoft.Fixture;

report 50340 "Navigation Report"
{
    ProcessingOnly = true;
    dataset
    {
        dataitem(Rows; "Navigation Row")
        {
            column(ID; ID) { }
            column(Value; Value) { }
        }
    }
    requestpage
    {
        layout
        {
            area(Content)
            {
                field(Limit; Limit) { }
            }
        }
    }
    var
        Limit: Integer;
}
