namespace Microsoft.Fixture;

pageextension 50272 "Added Page Controls" extends "Extension Controls"
{
    layout
    {
        addafter(Later) { field(Dependent; Value) { } }
        addlast(Fields) { field(Later; Value) { } }
        addfirst(Fields) { field(First; Value) { } }
        addbefore(Original) { field(Before; Value) { } }
        addafter(Original) { field(After; Value) { } }
        modify(Original) { Caption = 'Changed caption'; Editable = false; }
        modify(Dependent) { Caption = 'Changed dependent'; }
    }
    actions
    {
        addafter(LaterAction) { action(DependentAction) { } }
        addlast(Processing) { action(LaterAction) { } }
        addfirst(Processing) { action(FirstAction) { } }
        addbefore(OriginalAction) { action(BeforeAction) { } }
        addafter(OriginalAction) { action(AfterAction) { } }
        modify(OriginalAction) { Caption = 'Changed action'; }
    }
}
