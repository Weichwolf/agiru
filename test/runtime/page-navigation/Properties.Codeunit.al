namespace Microsoft.Fixture;

codeunit 50341 "Navigation Property Consumer"
{
    procedure Configure(var Subject: Report "Navigation Report"; Enabled: Boolean): Boolean
    begin
        Subject.UseRequestPage := Enabled;
        exit(Subject.UseRequestPage);
    end;
}
