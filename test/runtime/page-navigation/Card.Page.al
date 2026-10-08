namespace Microsoft.Fixture;

page 50340 "Navigation Card"
{
    PageType = Card;
    SourceTable = "Navigation Row";
    layout
    {
        area(Content)
        {
            field(ID; Rec.ID) { }
            field(Value; Rec.Value) { }
            field(OpeningMode; OpeningMode) { }
            field(LoadedValue; LoadedValue) { }
            field(Open; OpeningMode) { }
            field(Move; LoadedValue) { }
            field(Declaration; LoadedValue) { }
            field(ControlValue; OpeningMode) { }
        }
    }
    trigger OnOpenPage()
    begin
        OpeningMode := CurrPage.Editable;
    end;
    trigger OnNewRecord(BelowxRec: Boolean)
    begin
        Rec.Value := 314;
    end;
    trigger OnAfterGetRecord()
    begin
        LoadedValue := Rec.Value;
    end;
    var
        OpeningMode: Boolean;
        LoadedValue: Integer;
}
