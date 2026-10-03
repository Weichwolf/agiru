namespace Microsoft.Fixture;

using System.Reflection;

page 50179 "Record Binding"
{
    SourceTable = Field;
    SourceTableTemporary = true;

    procedure Choose(): Integer
    begin
        Rec.Class := Rec.Class::FlowFilter;
        exit(Rec.Class);
    end;

    procedure ChooseWithShadow(): Integer
    var
        Class: Option Alpha,Beta;
    begin
        Class := Class::Beta;
        Rec.Class := Rec.Class::FlowField;
        if Class <> Class::Beta then
            Error('The local option was changed');
        exit(Rec.Class);
    end;

    procedure ChoosePrevious(): Integer
    begin
        Rec.Class := xRec.Class::Normal;
        exit(Rec.Class);
    end;

    procedure NativeType(Choice: Integer): Integer
    begin
        case Choice of
            0: Rec.Type := Rec.Type::TableFilter;
            1: Rec.Type := Rec.Type::RecordID;
            2: Rec.Type := Rec.Type::OemText;
            3: Rec.Type := Rec.Type::Date;
            4: Rec.Type := Rec.Type::Time;
            5: Rec.Type := Rec.Type::DateFormula;
            6: Rec.Type := Rec.Type::Decimal;
            7: Rec.Type := Rec.Type::Media;
            8: Rec.Type := Rec.Type::MediaSet;
            9: Rec.Type := Rec.Type::Text;
            10: Rec.Type := Rec.Type::Code;
            11: Rec.Type := Rec.Type::Binary;
            12: Rec.Type := Rec.Type::BLOB;
            13: Rec.Type := Rec.Type::Boolean;
            14: Rec.Type := Rec.Type::Integer;
            15: Rec.Type := Rec.Type::OemCode;
            16: Rec.Type := Rec.Type::Option;
            17: Rec.Type := Rec.Type::BigInteger;
            18: Rec.Type := Rec.Type::Duration;
            19: Rec.Type := Rec.Type::GUID;
            20: Rec.Type := Rec.Type::DateTime;
            else Error('Unknown native type');
        end;
        exit(Rec.Type);
    end;

    procedure NativeProperties(): Integer
    var
        Other: Record Field temporary;
    begin
        Rec.SQLDataType := Rec.SQLDataType::BigInteger;
        Rec.Access := Rec.Access::Local;
        Rec.DataClassification := Rec.DataClassification::CustomerContent;
        Rec.ObsoleteState := Rec.ObsoleteState::Removed;
        Other.DataClassification := Other.DataClassification::SystemMetadata;
        Other.Type := Other.Type::Code;
        Rec.ExternalName := 'native_external';
        Rec.OptimizeForTextSearch := true;
        Rec.IsAllowedInCustomizations := true;
        exit(Other.Type + Other.DataClassification + Rec.SQLDataType + Rec.Access +
             Rec.DataClassification + Rec.ObsoleteState);
    end;

    procedure MetadataProperties(): Integer
    var
        PageMetadata: Record "Page Metadata" temporary;
        TableMetadata: Record "Table Metadata" temporary;
    begin
        PageMetadata.ID := 50175;
        PageMetadata.Name := 'Page source name';
        PageMetadata.Caption := 'Independent page caption';
        PageMetadata.PageType := PageMetadata.PageType::HeadlinePart;
        PageMetadata."DataCaptionExpr." := 'Source expression';
        PageMetadata.APIPublisher := 'Publisher';
        PageMetadata."AL Namespace" := 'Fixture.Pages';
        PageMetadata.Insert();
        PageMetadata.Name := 'Discarded page name';
        if not PageMetadata.Get(50175) then
            Error('The source page key was lost');
        if (PageMetadata.Name <> 'Page source name') or
           (PageMetadata.Caption <> 'Independent page caption') or
           (PageMetadata."DataCaptionExpr." <> 'Source expression') or
           (PageMetadata.APIPublisher <> 'Publisher') or
           (PageMetadata."AL Namespace" <> 'Fixture.Pages') then
            Error('Page Metadata source fields were lost');

        TableMetadata.ID := 50176;
        TableMetadata.Name := 'Table source name';
        TableMetadata.Caption := 'Independent table caption';
        TableMetadata.TableType := TableMetadata.TableType::Query;
        TableMetadata.ObsoleteState := TableMetadata.ObsoleteState::Removed;
        TableMetadata.DataClassification := TableMetadata.DataClassification::SystemMetadata;
        TableMetadata.CompressionType := TableMetadata.CompressionType::Page;
        TableMetadata.Scope := TableMetadata.Scope::OnPrem;
        TableMetadata.Access := TableMetadata.Access::Internal;
        TableMetadata.InherentPermissions := 'rX';
        TableMetadata."AL Namespace" := 'Fixture.Tables';
        TableMetadata.Insert();
        TableMetadata.Caption := 'Discarded table caption';
        if not TableMetadata.Get(50176) then
            Error('The implicit table key was lost');
        if (TableMetadata.Name <> 'Table source name') or
           (TableMetadata.Caption <> 'Independent table caption') or
           (TableMetadata.InherentPermissions <> 'rX') or
           (TableMetadata."AL Namespace" <> 'Fixture.Tables') then
            Error('Table Metadata source fields were lost');
        if (PageMetadata.Count <> 1) or (TableMetadata.Count <> 1) then
            Error('Temporary metadata rows leaked');
        exit(PageMetadata.PageType + TableMetadata.TableType + TableMetadata.ObsoleteState +
             TableMetadata.DataClassification + TableMetadata.CompressionType +
             TableMetadata.Scope + TableMetadata.Access);
    end;

    procedure Read(var Target: Text)
    begin
        Target := Rec.TableName;
    end;

    procedure SelectGroup(Group: Integer): Integer
    var
        Original: Integer;
    begin
        Original := Rec.FilterGroup;
        Rec.FilterGroup := Group;
        if Rec.FilterGroup <> Group then
            Error('The filter group did not change');
        exit(Original);
    end;

    procedure CurrentGroup(): Integer
    begin
        exit(FilterGroup);
    end;

    procedure PreviousGroup(): Integer
    begin
        exit(xRec.FilterGroup);
    end;

    procedure RowCount(): Integer
    begin
        exit(Rec.Count);
    end;

    procedure TemporaryStatus(): Boolean
    begin
        exit(Rec.IsTemporary);
    end;

    procedure SetNameFilter(Value: Text)
    begin
        Rec.SetRange(Rec.TableName, Value);
    end;

    procedure ReadNameFilter(): Text
    begin
        exit(Rec.GetFilter(Rec.TableName));
    end;

    procedure LocalNameFilter(): Text
    var
        Other: Record Field temporary;
    begin
        Other.SetRange(TableName, 'Local');
        exit(Other.GetFilter(TableName));
    end;

    procedure IgnoreLargeGroup(): Integer
    begin
        Rec.FilterGroup(256);
        exit(Rec.FilterGroup);
    end;

    procedure ReflectedGroups(): Integer
    var
        Ref: RecordRef;
        NameField: FieldRef;
        Original: Integer;
        Candidate: Integer;
    begin
        Ref.GetTable(Rec);
        NameField := Ref.Field(3);
        Ref.FilterGroup := 2;
        NameField.SetFilter('Ref two');
        Original := Ref.FilterGroup;
        if Ref.FilterGroup() <> Original then
            Error('The reflected getter changed the group');
        Ref.FilterGroup(256);
        if Ref.FilterGroup <> 2 then
            Error('The reflected group limit was ignored');
        if not Ref.HasFilter then
            Error('The reflected group lost its filter');
        NameField.SetRange();
        if Ref.HasFilter() then
            Error('The reflected filter was not cleared');
        Ref.FilterGroup(0);
        if not Ref.HasFilter then
            Error('Clearing group two deleted group zero');
        Ref.FilterGroup(10);
        NameField.SetFilter('Occupied');
        Candidate := 10;
        while Ref.HasFilter do begin
            Candidate += 1;
            if Candidate > 12 then
                Error('The free-group search did not terminate');
            Ref.FilterGroup(Candidate);
        end;
        exit(Ref.FilterGroup);
    end;
}
