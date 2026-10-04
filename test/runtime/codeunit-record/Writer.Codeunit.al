namespace Microsoft.Fixture;

codeunit 50261 "Run Writer"
{
    TableNo = "Run Row";

    trigger OnRun()
    var
        Stored: Record "Run Buffer";
    begin
        Stored.ID := Rec.ID;
        Stored.Value := Rec.Value;
        Stored.Insert();
        Rec.Save(Rec.Value);
        if Rec.Value < 0 then
            Error('');
        Rec.Value += 1;
    end;
}
