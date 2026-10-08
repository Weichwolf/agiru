namespace Microsoft.Fixture;

xmlport 50264 "Import Validation Consumer"
{
    Direction = Both;
    Format = Xml;
    UseRequestPage = false;
    DefaultFieldsValidation = false;
    schema
    {
        textelement(Root)
        {
            tableelement(Row; "Import Validation Record")
            {
                AutoSave = false;
                UseTemporary = false;
                fieldattribute(ID; Row.ID) { FieldValidate = no; }
                fieldattribute(Other; Row.Other)
                {
                    FieldValidate = Undefined;
                    trigger OnAfterAssignField()
                    begin
                        ObservedOther := Row.Other;
                        Row.Other += 2;
                    end;
                }
                fieldelement(Value; Row.Value)
                {
                    FieldValidate = yes;
                    trigger OnAfterAssignField()
                    begin
                        ObservedValue := Row.Value;
                        Row.Value += 1;
                    end;
                }
                fieldelement(Raw; Row.Raw)
                {
                    FieldValidate = no;
                    trigger OnAfterAssignField()
                    begin
                        ObservedRaw := Row.Raw;
                        Row.Raw += 3;
                    end;
                }
                trigger OnBeforeInsertRecord()
                begin
                    FinalValue := Row.Value;
                    FinalOther := Row.Other;
                    FinalRaw := Row.Raw;
                    ValidationCount := Row."Validation Count";
                    ValidatedValue := Row."Validated Value";
                    ValidatedOther := Row."Validated Other";
                    BeforeInsertCount += 1;
                end;
            }
        }
    }
    var
        ObservedValue: Integer;
        ObservedOther: Integer;
        ObservedRaw: Integer;
        FinalValue: Integer;
        FinalOther: Integer;
        FinalRaw: Integer;
        ValidationCount: Integer;
        ValidatedValue: Integer;
        ValidatedOther: Integer;
        BeforeInsertCount: Integer;

    procedure ValueObserved(): Integer
    begin
        exit(ObservedValue);
    end;

    procedure SetImportProperties(Name: Text; Importing: Boolean)
    begin
        CurrXmlPort.Filename := Name;
        CurrXmlPort.ImportFile := Importing;
    end;

    procedure PropertyFilename(): Text
    begin
        exit(CurrXmlPort.Filename);
    end;

    procedure SetExternalImportProperties(var OtherPort: XmlPort "Import Validation Consumer"; Name: Text; Importing: Boolean): Boolean
    begin
        OtherPort.Filename := Name;
        OtherPort.ImportFile := Importing;
        exit(OtherPort.ImportFile);
    end;

    procedure PropertyImporting(): Boolean
    begin
        exit(CurrXmlPort.ImportFile);
    end;
    procedure OtherObserved(): Integer
    begin
        exit(ObservedOther);
    end;
    procedure RawObserved(): Integer
    begin
        exit(ObservedRaw);
    end;
    procedure ValueFinal(): Integer
    begin
        exit(FinalValue);
    end;
    procedure OtherFinal(): Integer
    begin
        exit(FinalOther);
    end;
    procedure RawFinal(): Integer
    begin
        exit(FinalRaw);
    end;
    procedure Validations(): Integer
    begin
        exit(ValidationCount);
    end;
    procedure ValueValidated(): Integer
    begin
        exit(ValidatedValue);
    end;
    procedure OtherValidated(): Integer
    begin
        exit(ValidatedOther);
    end;
    procedure InsertBoundaries(): Integer
    begin
        exit(BeforeInsertCount);
    end;

    procedure ValidateValue(NewValue: Integer)
    begin
        Row.Validate(Value, NewValue);
    end;
}
