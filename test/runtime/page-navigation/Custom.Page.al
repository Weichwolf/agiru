namespace Microsoft.Fixture;

page 50355 "Navigation Custom"
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
                field(FindCalls; FindCalls) { }
                field(NextCalls; NextCalls) { }
                field(LastWhich; LastWhich) { }
                field(LastSteps; LastSteps) { }
                field(Loaded; Loaded) { }
                field(Original; Original) { }
                field(CurrentCount; CurrentCount) { }
            }
        }
    }
    trigger OnFindRecord(Which: Text): Boolean
    var
        Found: Boolean;
    begin
        FindCalls += 1;
        LastWhich := Which;
        if RefuseFind then
            exit(false);
        if UseTemporary then begin
            TempRows.Copy(Rec);
            Found := TempRows.Find(Which);
            if Found then
                Rec := TempRows;
            exit(Found);
        end;
        Rec.SetRange(ID, 2);
        exit(Rec.Find(Which));
    end;
    trigger OnNextRecord(Steps: Integer): Integer
    var
        Actual: Integer;
    begin
        NextCalls += 1;
        LastSteps := Steps;
        if FailNext then begin
            Rec.Value := 999;
            Rec.Modify();
            Error('custom next failed');
        end;
        if RefuseNext then
            exit(0);
        if UseTemporary then begin
            TempRows.Copy(Rec);
            Actual := TempRows.Next(Steps);
            if Actual <> 0 then
                Rec := TempRows;
            exit(Actual);
        end;
        exit(Rec.Next(Steps));
    end;
    trigger OnAfterGetRecord()
    begin
        Original := xRec.Value;
        Loaded := Rec.Value * 2;
    end;
    trigger OnAfterGetCurrRecord()
    begin
        CurrentCount += 1;
    end;
    procedure Configure(TemporarySource: Boolean; FindRefusal: Boolean; NextRefusal: Boolean; NextFailure: Boolean)
    begin
        UseTemporary := TemporarySource;
        RefuseFind := FindRefusal;
        RefuseNext := NextRefusal;
        FailNext := NextFailure;
        TempRows.ID := 7;
        TempRows.Value := 70;
        TempRows.Insert();
        TempRows.ID := 9;
        TempRows.Value := 90;
        TempRows.Insert();
    end;
    var
        TempRows: Record "Navigation Row" temporary;
        UseTemporary: Boolean;
        RefuseFind: Boolean;
        RefuseNext: Boolean;
        FailNext: Boolean;
        FindCalls: Integer;
        NextCalls: Integer;
        LastWhich: Text;
        LastSteps: Integer;
        Loaded: Integer;
        Original: Integer;
        CurrentCount: Integer;
}
