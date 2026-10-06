namespace Microsoft.Fixture;

table 50340 "Navigation Row"
{
    fields
    {
        field(1; ID; Integer) { }
        field(2; Value; Integer) { }
    }
    keys
    {
        key(PK; ID) { Clustered = true; }
    }

    procedure ReportRequestFlag(): Boolean
    var
        Subject: Report "Navigation Report";
    begin
        Subject.UseRequestPage := false;
        exit(Subject.UseRequestPage);
    end;
}
