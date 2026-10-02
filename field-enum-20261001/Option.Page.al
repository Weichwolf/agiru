namespace Microsoft.Fixture;
using System.Reflection;

page 50221 "Native Option"
{
    PageType = List;
    SourceTable = AllObjWithCaption;
    layout { area(Content) { field(ObjectType; Rec."Object Type") { ApplicationArea = All; } } }
    actions
    {
        area(Processing)
        {
            action(Select)
            {
                ApplicationArea = All;
                trigger OnAction()
                begin
                    Rec."Object Type" := Rec."Object Type"::Table;
                end;
            }
        }
    }
    trigger OnOpenPage()
    begin
        Rec.SetRange("Object Type", Rec."Object Type"::Report);
    end;
    procedure Exercise(): Integer
    var
        Ordinal: Integer;
    begin
        Rec."Object Type" := Rec."Object Type"::Report;
        if Rec."Object Type" <> Rec."Object Type"::Report then Error('Native explicit scope');
        Ordinal := Rec."Object Type";
        if Ordinal <> 3 then Error('Native ordinal');
        if Format(Rec."Object Type") <> 'Report' then Error('Native format');
        Rec.SetRange("Object Type", Rec."Object Type"::Report);
        Ordinal := Rec.GetRangeMin("Object Type");
        if Ordinal <> 3 then Error('Native filter lower bound');
        exit(4);
    end;
}
