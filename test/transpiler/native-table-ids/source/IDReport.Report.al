namespace Microsoft.Fixture;

report 50304 "ID Report"
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
