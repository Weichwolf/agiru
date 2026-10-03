namespace Microsoft.Fixture;

codeunit 50242 "Native Consumer UT" implements "Native Contract"
{
    Subtype = Test;

    procedure Echo(var Value: Enum "Native Sparse"): Enum "Native Sparse"
    var
        LocalValue: Enum "Native Sparse";
    begin
        LocalValue := Value;
        Value := Value::Extra;
        exit(LocalValue);
    end;

    procedure Numeric(Value: Enum 50240): Integer
    begin
        exit(Value.AsInteger());
    end;

    procedure Dispatch(var Value: Enum "Native Sparse"): Enum "Native Sparse"
    var
        Consumer: Codeunit "Native Consumer UT";
        Face: Interface "Native Contract";
    begin
        Face := Consumer;
        exit(Face.Echo(Value));
    end;

    [Test]
    procedure Kept()
    var
        Value: Enum "Native Sparse";
    begin
        Value := Value::Chosen;
        if Value.AsInteger() <> 10 then
            Error('Native enum ordinal was not preserved');
    end;
}
