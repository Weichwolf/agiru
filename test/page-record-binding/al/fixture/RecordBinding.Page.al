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
}
