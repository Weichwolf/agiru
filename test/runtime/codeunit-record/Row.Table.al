namespace Microsoft.Fixture;

table 50260 "Run Row"
{
    fields
    {
        field(1; ID; Integer) { }
        field(2; Value; Integer) { }
    }
    keys { key(PK; ID) { Clustered = true; } }

    var
        SavedRows: Record "Run Buffer" temporary;

    procedure Save(NewValue: Integer)
    begin
        SavedRows.Init();
        SavedRows.ID := SavedRows.Count() + 1;
        SavedRows.Value := NewValue;
        SavedRows.Insert();
    end;

    procedure SavedCount(): Integer
    begin
        exit(SavedRows.Count());
    end;

    procedure Restore()
    var
        Stored: Record "Run Buffer";
    begin
        if SavedRows.FindSet() then
            repeat
                Stored := SavedRows;
                Stored.Insert();
            until SavedRows.Next() = 0;
    end;

    procedure SavedValue(Number: Integer): Integer
    begin
        SavedRows.Get(Number);
        exit(SavedRows.Value);
    end;
}
