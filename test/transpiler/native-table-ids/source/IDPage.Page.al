namespace Microsoft.Fixture;

page 50303 "ID Page"
{
    procedure NativeID(): Integer
    begin
        exit(Database::"Declared Only");
    end;

    procedure QualifiedID(): Integer
    begin
        exit(Database::System.Fixture."Declared Only");
    end;
}
