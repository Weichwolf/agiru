namespace Microsoft.Fixture;

using System.Environment.Configuration;

codeunit 50232 "Saved Options"
{
    procedure Exercise(): Integer
    var
        Options: Record "Object Options";
        MemoryOptions: Record "Object Options" temporary;
        Flag: Record "Stored Flag" temporary;
        Ordinal: Integer;
        Output: OutStream;
        Input: InStream;
        Payload: Text;
    begin
        Options."Object Type" := Options."Object Type"::Report;
        Ordinal := Options."Object Type";
        if Ordinal <> 3 then Error('Report ordinal');
        Options."Object Type" := Options."Object Type"::XMLport;
        Ordinal := Options."Object Type";
        if Ordinal <> 6 then Error('XMLport ordinal');
        Options."Object Type" := Options."Object Type"::Page;
        Ordinal := Options."Object Type";
        if Ordinal <> 8 then Error('Page ordinal');
        Options.Temporary := true;
        if not Options.Temporary or Options.IsTemporary() then Error('Stored flag');
        MemoryOptions.Temporary := true;
        if not MemoryOptions.Temporary or not MemoryOptions.IsTemporary() then Error('Native temporary flag');
        Flag.Temporary := true;
        if not Flag.Temporary or not Flag.IsTemporary() then Error('Ordinary temporary flag');
        Options."Option Data".CreateOutStream(Output, TextEncoding::UTF8);
        Output.WriteText('Previous content that must disappear');
        Clear(Options."Option Data");
        Options."Option Data".CreateOutStream(Output, TextEncoding::UTF8);
        Output.WriteText('München 中文');
        Options."Option Data".CreateInStream(Input, TextEncoding::UTF8);
        Input.ReadText(Payload);
        if Payload <> 'München 中文' then Error('UTF8 replacement payload');
        Options."Object Type" := Options."Object Type"::Report;
        if Format(Options."Object Type") <> 'Report' then Error('Report format');
        Options.SetRange("Object Type", Options."Object Type"::Report);
        Ordinal := Options.GetRangeMin("Object Type");
        if Ordinal <> 3 then Error('Typed report range');
        exit(9);
    end;
}
