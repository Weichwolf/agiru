namespace Microsoft.Fixture;

table 50222 "Ordinary Option"
{
    DataClassification = SystemMetadata;
    fields
    {
        field(1; ID; Integer) {}
        field(2; State; Option) { OptionMembers = None,Ready; }
    }
    keys { key(PK; ID) { Clustered = true; } }
    procedure Exercise(): Integer
    var
        Ordinal: Integer;
    begin
        State := State::Ready;
        if State <> Rec.State::Ready then Error('Current option');
        Ordinal := Rec.State;
        if Ordinal <> 1 then Error('Ready ordinal');
        if xRec.State <> xRec.State::None then Error('Previous option');
        if Format(State) <> 'Ready' then Error('Ready format');
        exit(4);
    end;
}
