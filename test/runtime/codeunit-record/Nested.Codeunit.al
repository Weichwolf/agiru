namespace Microsoft.Fixture;

codeunit 50262 "Run Nested"
{
    TableNo = "Run Row";

    trigger OnRun()
    var
        Writer: Codeunit "Run Writer";
    begin
        Rec.Save(100);
        if not Writer.Run(Rec) then
            Rec.Save(200);
    end;
}
