namespace Microsoft.Fixture;

codeunit 50181 "Native Field"
{
    procedure Exercise(): Integer
    var
        Metadata: Record Field temporary;
    begin
        Metadata.TableNo := 50180;
        Metadata."No." := 1;
        Metadata.Type := Metadata.Type::Code;
        Metadata.ExternalName := 'external_code';
        Metadata.SQLDataType := Metadata.SQLDataType::BigInteger;
        Metadata.DataClassification := Metadata.DataClassification::SystemMetadata;
        Metadata.OptimizeForTextSearch := true;
        Metadata.Access := Metadata.Access::Protected;
        Metadata.IsAllowedInCustomizations := true;
        Metadata.Insert();
        if not Metadata.Get(50180, 1) then
            Error('Temporary native Field row is missing');
        if Metadata.Type <> 31489 then
            Error('Field.Type used an internal metadata tag');
        if Metadata.SQLDataType <> 3 then
            Error('SQLDataType ordinal is wrong');
        if Metadata.DataClassification <> 6 then
            Error('DataClassification uses telemetry ordering');
        if Metadata.Access <> 2 then
            Error('Access ordinal is wrong');
        if Metadata.ExternalName <> 'external_code' then
            Error('ExternalName was lost');
        if not Metadata.OptimizeForTextSearch then
            Error('Text search flag was lost');
        if not Metadata.IsAllowedInCustomizations then
            Error('Customization flag was lost');
        if Metadata.FieldNo("App Package ID") <> 60 then
            Error('Package field number is wrong');
        if Metadata.FieldNo("App Runtime Package ID") <> 61 then
            Error('Runtime package field number is wrong');
        Metadata.Type := Metadata.Type::BLOB;
        if Metadata.Type <> 33793 then
            Error('BLOB ordinal is wrong');
        Metadata.Type := Metadata.Type::GUID;
        if Metadata.Type <> 37119 then
            Error('GUID ordinal is wrong');
        Metadata.Type := Metadata.Type::RecordID;
        if Metadata.Type <> 4988 then
            Error('RecordID ordinal is wrong');
        exit(13);
    end;
}
