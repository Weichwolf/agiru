namespace Microsoft.Fixture;

page 50356 "Navigation Provider Window"
{
    PageType = List;
    Editable = false;
    SourceTable = "Navigation Row";
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
                field(FindCalls; FindCalls) { }
                field(NextCalls; NextCalls) { }
            }
        }
    }
    trigger OnOpenPage()
    begin
        Trace := 'O';
    end;
    trigger OnFindRecord(Which: Text): Boolean
    var
        Found: Boolean;
    begin
        FindCalls += 1;
        if RefuseFind then
            exit(false);
        if not UseTemporary then
            exit(Rec.Find(Which));
        TempRows.Copy(Rec);
        Found := TempRows.Find(Which);
        if Found then
            Rec := TempRows;
        exit(Found);
    end;
    trigger OnNextRecord(Steps: Integer): Integer
    var
        Actual: Integer;
    begin
        NextCalls += 1;
        if FailNext then begin
            Rec.Value := 999;
            Rec.Modify();
            Error('provider window next failed');
        end;
        if RefuseNext then
            exit(0);
        if StallNext then
            exit(Steps);
        if not UseTemporary then
            exit(Rec.Next(Steps));
        TempRows.Copy(Rec);
        Actual := TempRows.Next(Steps);
        if Actual <> 0 then
            Rec := TempRows;
        exit(Actual);
    end;
    trigger OnAfterGetRecord()
    begin
        ReadCount += 1;
        Original := xRec.Value;
        Loaded := Rec.Value * 2;
        if MutateBuffer then begin
            Rec.ID += 1000;
            Rec.Value := Loaded;
        end;
        Trace += 'A';
    end;
    trigger OnAfterGetCurrRecord()
    begin
        SelectedOriginal := xRec.Value;
        CurrentCount += 1;
        Trace += 'C';
    end;
    procedure Configure(TemporarySource: Boolean; Population: Integer; Mutate: Boolean)
    var
        Index: Integer;
    begin
        UseTemporary := TemporarySource;
        MutateBuffer := Mutate;
        for Index := 1 to Population do begin
            TempRows.ID := Index;
            TempRows.Value := Index * 10;
            TempRows.Insert();
        end;
    end;
    procedure ConfigureFailure(FindRefusal: Boolean; NextRefusal: Boolean; NextStall: Boolean; NextFailure: Boolean)
    begin
        RefuseFind := FindRefusal;
        RefuseNext := NextRefusal;
        StallNext := NextStall;
        FailNext := NextFailure;
    end;
    var
        TempRows: Record "Navigation Row" temporary;
        UseTemporary: Boolean;
        MutateBuffer: Boolean;
        RefuseFind: Boolean;
        RefuseNext: Boolean;
        StallNext: Boolean;
        FailNext: Boolean;
        Loaded: Integer;
        Original: Integer;
        SelectedOriginal: Integer;
        ReadCount: Integer;
        CurrentCount: Integer;
        FindCalls: Integer;
        NextCalls: Integer;
        Trace: Text;
}
