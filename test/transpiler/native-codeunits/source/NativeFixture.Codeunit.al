namespace System.Fixture;

codeunit 50311 NativeFixture
{
    [Native]
    procedure Empty()
    begin
    end;

    [Native]
    procedure Read(Value: Integer): Integer
    begin
        exit(Value);
    end;

    [nAtIvE]
    procedure Read(Value: Text): Text
    begin
        exit(Value);
    end;

    [Native]
    procedure Named() Result: Text
    var
        UnexpectedLocal: Integer;
    begin
        Result := 'successful fallback';
    end;

    [Native]
    procedure Write(var Value: Integer; Output: OutStream)
    begin
        Value := 99;
        Output.WriteText('mutation');
    end;

    [Native]
    [IntegrationEvent(false, false)]
    procedure Publish(var Value: Integer)
    begin
    end;

    [Native]
    local procedure Hidden()
    begin
    end;

    procedure CallHidden()
    begin
        Hidden();
    end;

    procedure OrdinaryEmpty()
    begin
    end;

    procedure Ordinary(): Integer
    begin
        exit(7);
    end;
}
