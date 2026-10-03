namespace Microsoft.Fixture;

report 50270 "Link Consumer"
{
    ProcessingOnly = true;

    dataset
    {
    }

    procedure LinkNativeEntrypoint()
    begin
        Original.Run();
    end;

    var
        Original: Report "Tenant Report Defaults";
}
