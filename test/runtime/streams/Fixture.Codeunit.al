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

    procedure InputFromLocalBlob(): InStream
    var
        LocalBlob: Blob;
        Output: OutStream;
        Input: InStream;
    begin
        LocalBlob.CreateOutStream(Output);
        Output.WriteText('local');
        LocalBlob.CreateInStream(Input);
        exit(Input);
    end;

    procedure OutputFromLocalBlob(var Input: InStream): OutStream
    var
        LocalBlob: Blob;
        Output: OutStream;
    begin
        LocalBlob.CreateInStream(Input);
        LocalBlob.CreateOutStream(Output);
        exit(Output);
    end;

    procedure WriteBlobByValue(Source: Blob): Integer
    var
        Output: OutStream;
    begin
        Source.CreateOutStream(Output);
        Output.WriteText('copy');
        exit(Source.Length());
    end;
}
