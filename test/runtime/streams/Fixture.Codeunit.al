namespace Microsoft.Fixture;

codeunit 50261 "Stream Alias Consumer"
{
    procedure ReadByValue(Source: InStream; var Value: Text; Count: Integer): Integer
    begin
        exit(Source.Read(Value, Count));
    end;

    procedure CopyAndRead(Source: InStream; var Value: Text): Integer
    var
        Assigned: InStream;
    begin
        Assigned := Source;
        exit(Assigned.Read(Value, 1));
    end;

    procedure ResetByValue(Source: InStream): Boolean
    begin
        exit(Source.ResetPosition());
    end;

    procedure Bind(var Source: InStream; var Store: Blob)
    begin
        Store.CreateInStream(Source);
    end;

    procedure ConsumeAndRebind(Source: InStream; var Value: Text; var Replacement: Blob)
    begin
        Source.Read(Value, 1);
        Replacement.CreateInStream(Source);
        Source.Read(Value, 1);
    end;
}
