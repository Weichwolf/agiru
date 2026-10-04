namespace Microsoft.Fixture;

codeunit 50323 "Native Consumer"
{
    var
        Bare: Codeunit "Source Native";
        Qualified: Codeunit System.Fixture."Source Native";
        Numeric: Codeunit 50321;

    procedure ByName(): Integer
    begin
        exit(Bare.Ordinary());
    end;

    procedure ByNamespace(): Integer
    begin
        exit(Qualified.Ordinary());
    end;

    procedure ById(): Integer
    begin
        exit(Numeric.Ordinary());
    end;

    procedure Unbound(var Value: Integer): Text
    begin
        exit(Qualified.Read(Value));
    end;
}
