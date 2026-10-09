namespace Fixture;
pageextension 50304 "Host Extension" extends "Feature Host"
{
    layout { addlast(content) { part(ExtensionCloud; "Remote Panel") { } } }
    procedure ExtensionFeature()
    begin CurrPage.ExtensionCloud.Page.Publish(Bump(8), Bump(9)); end;
}
