namespace Microsoft.Fixture;

page 50354 "Navigation Created"
{
    PageType = Card;
    SourceTable = "Navigation Row";
    layout
    {
        area(Content)
        {
            field(ID; Rec.ID) { }
            field(Value; Rec.Value)
            {
                trigger OnValidate()
                begin
                    if CreationMode = 3 then begin
                        Rec.Insert();
                        CreationMode := 0;
                    end;
                end;
            }
        }
    }
    trigger OnNewRecord(BelowxRec: Boolean)
    begin
        Rec.ID := NewID;
        Rec.Value := 17;
        if CreationMode = 1 then
            Rec.Insert();
    end;
    trigger OnAfterGetCurrRecord()
    var
        Created: Record "Navigation Row";
    begin
        if CreationMode <> 2 then
            exit;
        Created.ID := NewID;
        Created.Value := 17;
        Created.Insert();
        Rec.Copy(Created);
        CreationMode := 0;
    end;
    procedure Configure(Mode: Integer; ID: Integer)
    begin
        CreationMode := Mode;
        NewID := ID;
    end;
    var
        CreationMode: Integer;
        NewID: Integer;
}
