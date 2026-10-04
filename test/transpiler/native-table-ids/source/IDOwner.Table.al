namespace Microsoft.Fixture;

table 50302 "ID Owner"
{
    fields { field(1; ID; Integer) {} }

    procedure NativeID(): Integer
    begin
        exit(Database::"Declared Only");
    end;

    procedure QualifiedID(): Integer
    begin
        exit(Database::System.Fixture."Declared Only");
    end;
}
