namespace Fixture;
page 50300 "Feature Host"
{
    PageType = Card;
    layout
    {
        area(content)
        {
            field(Value; Value) { }
            part(Cloud; Fixture.Cloud."Remote Panel") { }
            part(CloudNumber; 50301) { }
            part(Required; "Business Chart") { }
            part(Missing; "Unselected ERP") { }
            usercontrol(Chart; BusinessChart) { }
        }
    }
    var Value: Integer;
    procedure OpenFeatures()
    begin
        Value := 1;
        CurrPage.Cloud.Page.Publish(Bump(2), Bump(3));
        CurrPage.Required.Page.Touch();
        if true then CurrPage.CloudNumber.Page.Publish(Bump(4), Bump(5));
        Value := Value + 10;
    end;
    procedure ConsumeCloud(): Integer
    begin exit(CurrPage.Cloud.Page.ReadValue(Bump(6))); end;
    procedure MissingERP()
    begin CurrPage.Missing.Page.Touch(Bump(7)); end;
    procedure UnknownAddIn()
    begin CurrPage.Chart.Refresh(Bump(8)); end;
    procedure Current(): Integer begin exit(Value); end;
    procedure DependencyValue(): Integer
    var Dependency: Codeunit Fixture.Cloud."Local Dependency";
    begin exit(Dependency.Value()); end;
    procedure UnavailableTextOperand(): Boolean
    var Service: Codeunit "Unavailable Service"; Content: Text;
    begin Content := 'ordinary'; exit(Content.Contains(Service.Token())); end;
    procedure UnavailableJsonOperand()
    var Service: Codeunit "Unavailable Service"; Content: JsonObject;
    begin Content.Add('enabled', Service.Enabled()); end;
    procedure RequiredTouches(): Integer begin exit(CurrPage.Required.Page.TouchCount()); end;
    procedure Bump(Number: Integer): Integer
    begin Value := Value * 10 + Number; exit(Value); end;
}
