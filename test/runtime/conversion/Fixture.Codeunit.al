namespace Microsoft.Fixture;

dotnet
{
    assembly(mscorlib)
    {
        type("System.Convert"; Convert) { }
        type("System.Array"; Array) { }
        type("System.Base64FormattingOptions"; Base64FormattingOptions) { }
    }
}

codeunit 50265 "Byte Conversion Consumer"
{
    procedure DecodeBigText(Input: BigText): DotNet Array
    var
        Convert: DotNet Convert;
    begin
        exit(Convert.FromBase64String(Input));
    end;

    procedure Encode(Input: DotNet Array): Text
    var
        Convert: DotNet Convert;
    begin
        exit(Convert.ToBase64String(Input));
    end;

    procedure EncodeRegion(Input: DotNet Array; Offset: Integer; Length: Integer; Formatting: Integer): Text
    var
        Convert: DotNet Convert;
        Options: DotNet Base64FormattingOptions;
    begin
        Options := Formatting;
        exit(Convert.ToBase64String(Input, Offset, Length, Options));
    end;

    procedure EncodeLines(Input: DotNet Array): Text
    var
        Convert: DotNet Convert;
        Options: DotNet Base64FormattingOptions;
    begin
        exit(Convert.ToBase64String(Input, Options.InsertLineBreaks));
    end;

    procedure Decode(Input: Text): DotNet Array
    var
        Convert: DotNet Convert;
    begin
        exit(Convert.FromBase64String(Input));
    end;

    procedure UnsupportedNumber(): Integer
    var
        Convert: DotNet Convert;
    begin
        exit(Convert.ToInt32(1));
    end;
}
