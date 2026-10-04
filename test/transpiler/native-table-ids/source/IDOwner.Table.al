namespace Microsoft.Fixture;

table 50302 "ID Owner"
{
    fields { field(1; ID; Integer) {} }

    procedure NativeID(): Integer
    begin
        exit(Database::"Declared Only");
    end;

    procedure OrdinaryID(): Integer
    begin
        exit(Database::"ID Owner");
    end;

    procedure QualifiedOrdinaryID(): Integer
    begin
        exit(Database::Microsoft.Fixture."ID Owner");
    end;

    procedure WrongOrdinaryNamespace(): Integer
    begin
        exit(Database::Other."ID Owner");
    end;

    procedure QualifiedID(): Integer
    begin
        exit(Database::System.Fixture."Declared Only");
    end;
}
