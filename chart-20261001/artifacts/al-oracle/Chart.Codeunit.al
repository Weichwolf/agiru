namespace Microsoft.Fixture;

using System.Reflection;

codeunit 50261 ChartRoundtrip
{
    procedure Exercise(): Integer
    var
        Source: Record Chart temporary;
        Target: Record Chart temporary;
        Ref: RecordRef;
        Writer: OutStream;
        Reader: InStream;
        Contents: Text;
    begin
        if Source.FieldNo(ID) <> 3 then Error('Chart ID field');
        if Source.FieldNo(Name) <> 6 then Error('Chart Name field');
        if Source.FieldNo(BLOB) <> 9 then Error('Chart Blob field');
        if MaxStrLen(Source.ID) <> 20 then Error('Chart ID length');
        if MaxStrLen(Source.Name) <> 30 then Error('Chart Name length');
        Ref.GetTable(Source);
        if Ref.Number() <> 2000000078 then Error('Chart source identity');
        if not Ref.FieldExist(3) then Error('Chart reflected ID');
        if not Ref.FieldExist(6) then Error('Chart reflected Name');
        if not Ref.FieldExist(9) then Error('Chart reflected Blob');
        if Ref.FieldExist(1) then Error('Chart cannot invent field 1');
        Source.ID := '  source  ';
        Source.Name := '  Mixed title  ';
        if Source.ID <> 'SOURCE' then Error('Chart Code normalization');
        if Source.Name <> '  Mixed title  ' then Error('Chart Text value');
        Source.BLOB.CreateOutStream(Writer, TextEncoding::UTF8);
        Writer.WriteText('<Chart Name="Ägirū"/>');
        Source.BLOB.CreateInStream(Reader, TextEncoding::UTF8);
        Reader.Read(Contents);
        if Contents <> '<Chart Name="Ägirū"/>' then Error('Chart UTF8 Blob');
        Source.Insert();
        Target := Source;
        Target.ID := 'copy';
        Target.Name := 'Independent title';
        if Source.ID <> 'SOURCE' then Error('Source ID independence');
        if Source.Name <> '  Mixed title  ' then Error('Source Name independence');
        Target.BLOB.CreateInStream(Reader, TextEncoding::UTF8);
        Reader.Read(Contents);
        if Contents <> '<Chart Name="Ägirū"/>' then Error('Copied Blob value');
        Clear(Target.BLOB);
        if Target.BLOB.HasValue() then Error('Target Blob Clear');
        if not Source.BLOB.HasValue() then Error('Source Blob independence');
        if not Target.IsTemporary() then Error('Target temporary ownership');
        Target.Insert();
        if Target.Count() <> 1 then Error('Target owns its row store');
        if Source.Count() <> 1 then Error('Source owns its row store');
        exit(21);
    end;
}
