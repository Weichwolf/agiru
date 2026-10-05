namespace Microsoft.Fixture;

dotnet
{
    assembly(mscorlib)
    {
        type("System.Security.Cryptography.HashAlgorithm"; HashAlgorithm) { }
        type("System.Array"; Array) { }
        type("System.Text.Encoding"; Encoding) { }
        type("System.Convert"; Convert) { }
    }
}

codeunit 50264 "Hash Consumer"
{
    procedure HashBase64(Input: Text; Algorithm: Text): Text
    var
        Convert: DotNet Convert;
    begin
        exit(Convert.ToBase64String(HashText(Input, Algorithm)));
    end;

    procedure HashText(Input: Text; Algorithm: Text): DotNet Array
    var
        Hash: DotNet HashAlgorithm;
        Encoding: DotNet Encoding;
        Result: DotNet Array;
    begin
        Hash := Hash.Create(Algorithm);
        Result := Hash.ComputeHash(Encoding.UTF8().GetBytes(Input));
        Hash.Dispose();
        exit(Result);
    end;

    procedure HashRegion(Input: DotNet Array; Algorithm: Text; Offset: Integer; Count: Integer): DotNet Array
    var
        Hash: DotNet HashAlgorithm;
        Result: DotNet Array;
    begin
        Hash := Hash.Create(Algorithm);
        Result := Hash.ComputeHash(Input, Offset, Count);
        Hash.Dispose();
        exit(Result);
    end;

    procedure DisposeByValue(Hash: DotNet HashAlgorithm)
    begin
        Hash.Dispose();
    end;

    procedure UnknownIsNull(): Boolean
    var
        Hash: DotNet HashAlgorithm;
    begin
        Hash := Hash.Create('unknown');
        exit(IsNull(Hash));
    end;
}
