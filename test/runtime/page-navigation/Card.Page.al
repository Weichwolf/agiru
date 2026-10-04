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
        }
    }
    trigger OnOpenPage()
    begin
        OpeningMode := CurrPage.Editable;
    end;
    trigger OnAfterGetRecord()
    begin
        LoadedValue := Rec.Value;
    end;
    var
        OpeningMode: Boolean;
        LoadedValue: Integer;
}
