namespace Fixture;
page 50302 "Business Chart"
{
    PageType = CardPart;
    var Count: Integer;
    procedure Touch() begin Count := Count + 1; end;
    procedure TouchCount(): Integer begin exit(Count); end;
}
