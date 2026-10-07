namespace Microsoft.Fixture;

page 50350 "Navigation Window"
{
    PageType = List;
    Editable = false;
    SourceTable = "Navigation Row";
    SourceTableView = sorting(ID) where(ID = filter(1..100));
    layout
    {
        area(Content)
        {
            repeater(Rows)
            {
                field(ID; Rec.ID) { }
                field(Value; Rec.Value) { }
                field(Loaded; Loaded) { }
                field(Original; Original) { }
                field(SelectedOriginal; SelectedOriginal) { }
                field(ReadCount; ReadCount) { }
                field(CurrentCount; CurrentCount) { }
                field(Trace; Trace) { }
            }
        }
    }
    trigger OnOpenPage()
    begin
        Trace := 'O';
    end;
    trigger OnAfterGetRecord()
    begin
        ReadCount += 1;
        Loaded := Rec.Value * 2;
        Original := xRec.Value;
        Rec.Value := Loaded;
        Trace += 'A';
    end;
    trigger OnAfterGetCurrRecord()
    begin
        SelectedOriginal := xRec.Value;
        CurrentCount += 1;
        Trace += 'C';
    end;
    var
        Loaded: Integer;
        Original: Integer;
        SelectedOriginal: Integer;
        ReadCount: Integer;
        CurrentCount: Integer;
        Trace: Text;
}
