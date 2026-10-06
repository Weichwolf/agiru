namespace Microsoft.Fixture;

codeunit 50261 "Stream Alias Consumer"
{
    procedure CultureName(CultureId: Integer): Text
    var
        Culture: DotNet CultureInfo;
    begin
        Culture := Culture.CultureInfo(CultureId);
        exit(Culture.Name);
    end;

    procedure BinaryRoundTrip(var Store: Blob; Value: Text): Text
    var
        Output: OutStream;
        Input: InStream;
        Writer: DotNet BinaryWriter;
        Reader: DotNet BinaryReader;
    begin
        Store.CreateOutStream(Output);
        Writer := Writer.BinaryWriter(Output);
        Writer.Write(Value);
        Store.CreateInStream(Input);
        Reader := Reader.BinaryReader(Input);
        exit(Reader.ReadString());
    end;

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

    procedure ReadDefaultFile(Name: Text; var Value: Text): Integer
    var
        Source: File;
        Count: Integer;
    begin
        Source.Open(Name);
        Count := Source.Read(Value);
        Source.Close();
        exit(Count);
    end;

    procedure ReadTextFile(Name: Text; var Value: Text): Integer
    var
        Source: File;
        Count: Integer;
    begin
        Source.TextMode(true);
        Source.Open(Name);
        Count := Source.Read(Value);
        Source.Close();
        exit(Count);
    end;

    procedure ReadBoundedFile(Name: Text; var Value: Text): Integer
    var
        Source: File;
        Bounded: Text[3];
        Count: Integer;
    begin
        Source.Open(Name);
        Count := Source.Read(Bounded);
        Value := Bounded;
        Source.Close();
        exit(Count);
    end;

    procedure FilePosition(Name: Text; Offset: Integer): Integer
    var
        Source: File;
        Position: Integer;
    begin
        Source.Open(Name);
        Source.Seek(Offset);
        Position := Source.Pos();
        Source.Close();
        exit(Position);
    end;
}
