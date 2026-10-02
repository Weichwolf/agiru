namespace Microsoft.Fixture;

using System.Reflection;

codeunit 50198 "Native Metadata"
{
    procedure Exercise(): Integer
    var
        Metadata: Record "Table Metadata" temporary;
        Ref: RecordRef;
        Application: Guid;
    begin
        Application := '33333333-3333-3333-3333-333333333333';
        Metadata.ID := 136;
        Metadata.Name := 'Original.AL.Name';
        Metadata.Caption := PadStr('', 80, 'c');
        Metadata.DataPerCompany := true;
        Metadata.LookupPageID := 10;
        Metadata.DrillDownPageId := 20;
        Metadata.DataCaptionFields := PadStr('', 80, 'd');
        Metadata.PasteIsValid := false;
        Metadata.LinkedObject := true;
        Metadata.DataIsExternal := true;
        Metadata.TableType := Metadata.TableType::Query;
        Metadata.ExternalName := PadStr('', 248, 'e');
        Metadata.ObsoleteState := Metadata.ObsoleteState::Removed;
        Metadata.ObsoleteReason := PadStr('', 248, 'r');
        Metadata.DataClassification := Metadata.DataClassification::SystemMetadata;
        Metadata.ReplicateData := false;
        Metadata.CompressionType := Metadata.CompressionType::Page;
        Metadata."App ID" := Application;
        Metadata.InherentPermissions := 'rIMDx';
        Metadata.InherentEntitlements := 'RimDX';
        Metadata.Scope := Metadata.Scope::OnPrem;
        Metadata.Access := Metadata.Access::Internal;
        Metadata."AL Namespace" := PadStr('', 500, 'n');
        Metadata.Insert();
        if not Metadata.Get(136) then Error('Temporary metadata lookup');
        if Metadata.ID <> 136 then Error('ID');
        if Metadata.Name <> 'Original.AL.Name' then Error('Name is not Caption');
        if Metadata.Caption <> PadStr('', 80, 'c') then Error('Caption');
        if not Metadata.DataPerCompany then Error('DataPerCompany');
        if Metadata.LookupPageID <> 10 then Error('LookupPageID');
        if Metadata.DrillDownPageId <> 20 then Error('DrillDownPageId');
        if Metadata.DataCaptionFields <> PadStr('', 80, 'd') then Error('DataCaptionFields');
        if Metadata.PasteIsValid then Error('PasteIsValid');
        if not Metadata.LinkedObject then Error('LinkedObject');
        if not Metadata.DataIsExternal then Error('DataIsExternal');
        if Metadata.TableType <> Metadata.TableType::Query then Error('Query');
        if Metadata.ExternalName <> PadStr('', 248, 'e') then Error('ExternalName');
        if Metadata.ObsoleteState <> Metadata.ObsoleteState::Removed then Error('Removed');
        if Metadata.ObsoleteReason <> PadStr('', 248, 'r') then Error('ObsoleteReason');
        if Metadata.DataClassification <> Metadata.DataClassification::SystemMetadata then Error('SystemMetadata');
        if Metadata.ReplicateData then Error('ReplicateData');
        if Metadata.CompressionType <> Metadata.CompressionType::Page then Error('Page');
        if Metadata."App ID" <> Application then Error('App ID');
        if Metadata.InherentPermissions <> 'rIMDx' then Error('InherentPermissions');
        if Metadata.InherentEntitlements <> 'RimDX' then Error('InherentEntitlements');
        if Metadata.Scope <> Metadata.Scope::OnPrem then Error('OnPrem');
        if Metadata.Access <> Metadata.Access::Internal then Error('Internal');
        if Metadata."AL Namespace" <> PadStr('', 500, 'n') then Error('AL Namespace');
        if Format(Metadata.TableType) <> 'Query' then Error('Query name');
        if Format(Metadata.ObsoleteState) <> 'Removed' then Error('Removed name');
        if Format(Metadata.DataClassification) <> 'SystemMetadata' then Error('Classifier name');
        if Format(Metadata.CompressionType) <> 'Page' then Error('Compression name');
        if Format(Metadata.Scope) <> 'OnPrem' then Error('Scope name');
        if Format(Metadata.Access) <> 'Internal' then Error('Access name');
        if Metadata.TableType.AsInteger() <> 5 then Error('Query position');
        if Metadata.ObsoleteState.AsInteger() <> 2 then Error('Removed position');
        if Metadata.DataClassification.AsInteger() <> 6 then Error('Classifier position');
        if Metadata.CompressionType.AsInteger() <> 3 then Error('Compression position');
        if Metadata.Scope.AsInteger() <> 1 then Error('Scope position');
        if Metadata.Access.AsInteger() <> 1 then Error('Access position');
        Ref.GetTable(Metadata);
        if Ref.FieldCount() <> 23 then Error('All source fields');
        if Ref.KeyCount() <> 1 then Error('Effective default key');
        if not Ref.FieldExist(23) then Error('Original namespace slot');
        exit(39);
    end;
}
