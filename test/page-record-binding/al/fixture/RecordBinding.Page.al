namespace Microsoft.Fixture;

using System.Reflection;

page 50179 "Record Binding"
{
    SourceTable = Field;
    SourceTableTemporary = true;

    procedure Choose(): Integer
    begin
        Rec.Class := Rec.Class::FlowFilter;
        exit(Rec.Class);
    end;

    procedure ChooseWithShadow(): Integer
    var
        Class: Option Alpha,Beta;
    begin
        Class := Class::Beta;
        Rec.Class := Rec.Class::FlowField;
        if Class <> Class::Beta then
            Error('The local option was changed');
        exit(Rec.Class);
    end;

    procedure ChoosePrevious(): Integer
    begin
        Rec.Class := xRec.Class::Normal;
        exit(Rec.Class);
    end;

    procedure Read(var Target: Text)
    begin
        Target := Rec.TableName;
    end;

    procedure SelectGroup(Group: Integer): Integer
    var
        Original: Integer;
    begin
        Original := Rec.FilterGroup;
        Rec.FilterGroup := Group;
        if Rec.FilterGroup <> Group then
            Error('The filter group did not change');
        exit(Original);
    end;

    procedure CurrentGroup(): Integer
    begin
        exit(FilterGroup);
    end;

    procedure PreviousGroup(): Integer
    begin
        exit(xRec.FilterGroup);
    end;

    procedure RowCount(): Integer
    begin
        exit(Rec.Count);
    end;

    procedure TemporaryStatus(): Boolean
    begin
        exit(Rec.IsTemporary);
    end;

    procedure SetNameFilter(Value: Text)
    begin
        Rec.SetRange(Rec.TableName, Value);
    end;

    procedure ReadNameFilter(): Text
    begin
        exit(Rec.GetFilter(Rec.TableName));
    end;

    procedure LocalNameFilter(): Text
    var
        Other: Record Field temporary;
    begin
        Other.SetRange(TableName, 'Local');
        exit(Other.GetFilter(TableName));
    end;
}
