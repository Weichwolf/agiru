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

    procedure UnavailableIndexedFields(Operation: Integer; var Counter: Integer)
    var
        Rows: array[2] of Record "Indexed Row";
        Grid: array[2,2] of Record "Indexed Row";
        Slots: array[2] of Integer;
        Value: Integer;
    begin
        Slots[1] := 1;
        Counter := 41;
        case Operation of
            0: Value := Rows[1]."Array Only";
            1: Value := Rows[Slots[1]]."Nested Only";
            2: Value := Grid[1,2]."Matrix Only";
            3: Value := Rows[ChooseIndex(Counter, ']')]."Bracket Only";
            4: Rows[1].Validate("Validated Only", Value);
        end;
        Counter := 99;
    end;

    local procedure ChooseIndex(var Counter: Integer; Marker: Text): Integer
    begin
        Counter += 1;
        exit(1);
    end;

    procedure UnavailableFieldOperations(Operation: Integer; var Counter: Integer)
    var
        Row: Record "Arithmetic Row";
    begin
        Counter := 41;
        case Operation of
            0: Row.Amount += 3;
            1: Row.Amount -= 3;
            2: Row.Amount *= 3;
            3: Row.Amount /= 3;
            4: Clear(Row.Amount);
            5: System.Clear(Row.Amount);
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

    procedure UnavailableFieldNumber(Operation: Integer; var Counter: Integer)
    var
        Row: Record "Number Row";
        Rows: array[2] of Record "Number Row";
        Grid: array[2,2] of Record "Number Row";
        Value: Integer;
    begin
        Counter := 41;
        case Operation of
            0: Value := FieldNumberResult(Row.FieldNo("Only Number Field"));
            1: Value := FieldNumberResult(Rows[1].FieldNo("Only Number Field"));
            2: Value := FieldNumberResult(Grid[1,2].FieldNo("Only Number Field"));
            3: Value := FieldNumberResult(Row.fIeLdNo(Row."Only Number Field"));
        end;
        Counter := 99;
    end;

    procedure AvailableFieldNumber(): Integer
    var
        Row: Record "Available Row" temporary;
    begin
        exit(FieldNumberResult(Row.FieldNo(ID)));
    end;

    procedure OrdinaryFieldNumberName(): Text
    var
        Peer: Codeunit "Number Peer";
    begin
        exit(Peer.FieldNo());
    end;

    local procedure FieldNumberResult(Value: Integer): Integer
    begin
        exit(Value);
    end;

    local procedure FieldNumberResult(Value: JsonArray): Integer
    begin
        exit(-1);
    end;

    procedure UnavailablePart(Operation: Integer; var Counter: Integer)
    var
        Host: TestPage "Part Host";
        Value: Boolean;
    begin
        Counter := 41;
        case Operation of
            0: Host.MissingPart.First();
            1: Value := Host.MissingPart.Name.Visible();
            2: Host.MissingPart.Opportunities.DrillDown();
            3: Host.MissingPart.Name.SetValue(PartArgument(Counter));
            4: Host.MissingPart.Page().Name.SetValue(PartArgument(Counter));
        end;
        Counter := 99;
    end;

    procedure UnavailablePartInUntakenBranch(): Integer
    var
        Host: TestPage "Part Host";
    begin
        if false then
            Host.MissingPart.First();
        exit(7);
    end;

    local procedure PartArgument(var Counter: Integer): Text
    begin
        Counter += 1;
        exit('argument');
    end;
}
