namespace Microsoft.Fixture;

using System.Integration;

codeunit 50251 "OData Roundtrip"
{
    procedure Exercise(): Integer
    var
        Definition: Record "OData Edm Type" temporary;
        Reflected: RecordRef;
        Writer: OutStream;
        Reader: InStream;
        Payload: Text;
    begin
        if not Definition.IsTemporary() then Error('Temporary record');
        if Definition.FieldNo("Key") <> 1 then Error('Original key field');
        if Definition.FieldNo(Description) <> 2 then Error('Original description field');
        if Definition.FieldNo("Edm Xml") <> 10 then Error('Original Blob field');
        Reflected.GetTable(Definition);
        if Reflected.Number() <> 2000000179 then Error('Original System identity');
        if not Reflected.FieldExist(10) or Reflected.FieldExist(3) then Error('Original reflected field identity');
        if Reflected.Field(10).Name() <> 'Edm Xml' then Error('Original reflected field name');
        Definition."Key" := 'SCHEMA';
        Definition.Description := 'Definition';
        if (Definition."Key" <> 'SCHEMA') or (Definition.Description <> 'Definition') then Error('Original Code and Text fields');
        if Definition."Edm Xml".HasValue() then Error('Initially empty Blob');
        Definition."Edm Xml".CreateOutStream(Writer, TextEncoding::UTF8);
        Writer.WriteText('<Schema Name="Ägirū"/>');
        if not Definition."Edm Xml".HasValue() then Error('Populated UTF8 Blob');
        Definition."Edm Xml".CreateInStream(Reader, TextEncoding::UTF8);
        Reader.Read(Payload);
        if Payload <> '<Schema Name="Ägirū"/>' then Error('Original UTF8 Blob read');
        Clear(Definition."Edm Xml");
        if Definition."Edm Xml".HasValue() then Error('Blob Clear');
        Definition."Edm Xml".CreateOutStream(Writer, TextEncoding::UTF8);
        Writer.WriteText('<B/>');
        Definition."Edm Xml".CreateInStream(Reader, TextEncoding::UTF8);
        Reader.Read(Payload);
        if Payload <> '<B/>' then Error('Shorter replacement has no old tail');
        exit(13);
    end;
}
