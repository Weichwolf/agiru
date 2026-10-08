namespace System.Fixture;

codeunit 50318 "Absent Peer"
{
    procedure Use(var Row: Record "case row"; var Counter: Integer)
    var
        Value: Integer;
    begin
        Counter += 1;
        Value := Row.CalleeOnly;
        Counter := 99;
    end;
}
