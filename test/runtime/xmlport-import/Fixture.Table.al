namespace Microsoft.Fixture;

table 50263 "Import Validation Record"
{
    fields
    {
        field(1; ID; Integer) { }
        field(2; Value; Integer)
        {
            trigger OnValidate()
            begin
                "Validation Count" += 1;
                "Validated Value" := Value;
                if Value = 99 then
                    Error('Rejected import value');
            end;
        }
        field(3; Other; Integer)
        {
            trigger OnValidate()
            begin
                "Validation Count" += 1;
                "Validated Other" := Other;
            end;
        }
        field(4; Raw; Integer)
        {
            trigger OnValidate()
            begin
                Error('Raw field must not be validated');
            end;
        }
        field(5; "Validation Count"; Integer) { }
        field(6; "Validated Value"; Integer) { }
        field(7; "Validated Other"; Integer) { }
    }
    keys
    {
        key(PK; ID) { Clustered = true; }
    }
}
