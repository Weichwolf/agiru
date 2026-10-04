namespace System.Fixture;

codeunit 50321 "Source Native"
{
    var
        Peer: Codeunit "Source Peer";

    procedure Ordinary(): Integer
    begin
        exit(Peer.Read());
    end;

    [Native]
    procedure Read(var Value: Integer): Text
    begin
        Value := 999;
        exit('must not execute');
    end;
}
