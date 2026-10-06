namespace Microsoft.Fixture;

table 50262 "Global Stream Reader Consumer"
{
    fields
    {
        field(1; ID; Integer) { }
    }
    var
        Reader: DotNet StreamReader;

    procedure ReadFirst(var Store: Blob): Text
    var
        Input: InStream;
    begin
        Store.CreateInStream(Input);
        Reader := Reader.StreamReader(Input);
        if Reader.EndOfStream then
            exit('empty');
        exit(Reader.ReadLine());
    end;
}
