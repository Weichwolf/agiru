namespace Microsoft.Fixture;

codeunit 50261 "Stream Alias Consumer"
{
    procedure ClrChart(Value: Decimal): Decimal
    var
        Table: DotNet DataTable;
        Column: DotNet DataColumn;
        Row: DotNet DataRow;
        Chart: DotNet BusinessChartData;
    begin
        Table := Table.DataTable();
        Column := Column.DataColumn('Amount');
        Table.Columns.Add(Column);
        Row := Table.NewRow();
        Row.Item('Amount', Value);
        Table.Rows.Add(Row);
        Chart := Chart.BusinessChartData();
        Chart.DataTable := Table;
        exit(Chart.DataTable.Rows.Item(0).Item('Amount'));
    end;

    procedure ClrQueue(Value: Text): Text
    var
        Queue: DotNet Queue;
    begin
        Queue := Queue.Queue();
        Queue.Enqueue(Value);
        exit(Queue.Dequeue());
    end;

    procedure ClrUri(Value: Text): Text
    var
        Address: DotNet Uri;
        Builder: DotNet UriBuilder;
    begin
        Address := Address.Uri(Value);
        Builder := Builder.UriBuilder(Address);
        Builder.Query := '?new=2';
        Address := Builder.Uri;
        exit(Address.AbsoluteUri);
    end;

    procedure ReadDecodedLines(var Store: Blob): Text
    var
        Input: InStream;
        Reader: DotNet StreamReader;
        Value: Text;
    begin
        Store.CreateInStream(Input);
        Reader := Reader.StreamReader(Input);
        if Reader.EndOfStream then
            exit('empty');
        Value := Reader.ReadLine();
        if not Reader.EndOfStream then
            Value += '|' + Reader.ReadToEnd();
        if not Reader.EndOfStream then
            Error('Decoded reader did not reach its end.');
        exit(Value);
    end;

    procedure ClrEncodingPages(): Integer
    var
        Base: DotNet Encoding;
        Utf8: DotNet UTF8Encoding;
        Utf16: DotNet UnicodeEncoding;
        Ascii: DotNet ASCIIEncoding;
    begin
        Base := Base.Encoding();
        Utf8 := Utf8.UTF8Encoding(true);
        Utf16 := Utf16.UnicodeEncoding();
        Ascii := Ascii.ASCIIEncoding();
        exit(Base.CodePage + Utf8.CodePage + Utf16.CodePage + Ascii.CodePage);
    end;

    procedure ClrString(Value: Text): Text
    var
        StringValue: DotNet String;
    begin
        StringValue := StringValue.String(Value);
        exit(StringValue.ToString());
    end;

    procedure ClrRegex(Value: Text): Text
    var
        Pattern: DotNet Regex;
    begin
        Pattern := Pattern.Regex('(?<word>[a-z]+)-(\d+)');
        exit(Pattern.Replace(Value, '${word}=$2'));
    end;

    procedure ClrTimeSpan(): BigInteger
    var
        Span: DotNet TimeSpan;
    begin
        Span := Span.TimeSpan(1, 2, 3);
        exit(Span.Ticks);
    end;

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
