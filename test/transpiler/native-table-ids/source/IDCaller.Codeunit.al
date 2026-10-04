namespace Microsoft.Fixture;

codeunit 50301 "ID Caller"
{
    procedure NativeID(): Integer
    begin
        exit(Database::"Declared Only");
    end;

    procedure QualifiedID(): Integer
    begin
        exit(database::System.Fixture."Declared Only");
    end;

    procedure WrongNamespace(): Integer
    begin
        exit(Database::Other."Declared Only");
    end;

    procedure MissingID(): Integer
    begin
        exit(Database::Missing);
    end;

    procedure OtherKind(): Integer
    begin
        exit(Page::"ID Page");
    end;

    procedure Shadowed(): Integer
    var
        Database: Option First,Second;
    begin
        exit(Database::Second);
    end;

    procedure NativeRead(): Boolean
    var
        Value: Record "Declared Only";
    begin
        exit(Value.FindFirst());
    end;
}
