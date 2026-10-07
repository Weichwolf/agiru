namespace Microsoft.Fixture;

table 50340 "Navigation Row"
{
    fields
    {
        field(1; ID; Integer) { }
        field(2; Value; Integer) { }
        field(3; Label; Text[80]) { }
        field(4; Amount; Decimal) { }
        field(5; Exact; BigInteger) { }
        field(6; Code; Code[20]) { }
    }
    keys
    {
        key(PK; ID) { Clustered = true; }
        key(ByCode; Code) { }
    }

    procedure ReportRequestFlag(): Boolean
    var
        Subject: Report "Navigation Report";
    begin
        Subject.UseRequestPage := false;
        exit(Subject.UseRequestPage);
    end;
}
