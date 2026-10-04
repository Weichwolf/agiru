namespace Microsoft.Fixture;

table 60003 "Owned Row"
{
    fields
    {
        field(1; ID; Integer) { }
        field(3; "Nested Value"; Text[20])
        {
            ObsoleteState = Moved;
            MovedTo = '118874ab-44bc-4ccb-9daf-59763539ab16';
        }
    }
}
