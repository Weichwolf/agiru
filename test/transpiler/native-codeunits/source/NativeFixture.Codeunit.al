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

    procedure UnavailableFields(Operation: Integer; var Counter: Integer)
    var
        Row: Record "Unavailable Row";
        Value: Boolean;
    begin
        Counter := 41;
        case Operation of
            0: Row.ModifyAll(Enabled, Value);
            1: Row.LoadFields("Quoted Field", Enabled);
            2: Row.GetRangeMin("Quoted Field");
            3: Row.GetRangeMax("Quoted Field");
            4: Row.GetFilter("Quoted Field");
            5: Row.GetAscending("Quoted Field");
            6: Row.CopyFilter("Quoted Field", Enabled);
            7: Row.FieldActive(Enabled);
            8: Row.Relation(Enabled);
            9: Row.AreFieldsLoaded("Quoted Field", Enabled);
        end;
        Counter := 99;
    end;

    procedure UnavailablePage(Operation: Integer; var Counter: Integer)
    var
        Row: Record "Available Row" temporary;
        Choice: Action;
    begin
        Counter := 41;
        case Operation of
            0: Page.Run(Page::"Unselected Page");
            1: Page.RunModal(Page::"Unselected Page");
            2: Choice := Page.RunModal(Page::"Unselected Page");
            3: Page.Run(Page::"Unselected Page", Row);
            4: Choice := Page.RunModal(Page::"Unselected Page", Row, Row.FieldNo(ID));
            5: page.rUn(page::"Unselected Page");
        end;
        Counter := 99;
    end;
}
