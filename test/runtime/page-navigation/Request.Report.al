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

    procedure SetRequestPageEnabled(Enabled: Boolean)
    begin
        CurrReport.UseRequestPage := Enabled;
    end;

    procedure RequestPageEnabled(): Boolean
    begin
        exit(CurrReport.UseRequestPage);
    end;
}
