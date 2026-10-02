namespace System.Reflection;

page 50171 "Native Field Binding Fixture"
{
    PageType = List;
    SourceTable = Field;

    layout
    {
        area(content)
        {
            repeater(Rows)
            {
                field(Number; Rec."No.") { }
                field(TypeValue; Rec.Type) { }
                field(ExternalName; Rec.ExternalName) { }
            }
        }
    }
}
