// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Reflection;

/// <summary>
/// Virtual table that provides metadata information about all fields in all tables within the database.
/// This table enables introspection of field definitions, properties, and characteristics across the entire data model.
/// </summary>
/// <remarks>
/// The Field table is a critical metadata source for data model analysis, field discovery, and schema inspection.
/// It contains detailed information about field types, relationships, data classification, and various field properties.
/// This table is extensively used by development tools, data migration utilities, configuration management,
/// and customization frameworks that need to understand the structure and constraints of table fields.
/// The information includes both base application fields and those added by extensions.
/// </remarks>
table 2000000041 "Field"
{
    Scope = Cloud;
    InherentPermissions = rX;

    fields
    {
        /// <summary>
        /// The table number that contains this field.
        /// </summary>
        field(1; TableNo; Integer)
        {
        }
        /// <summary>
        /// The unique field number within the table.
        /// </summary>
        field(2; "No."; Integer)
        {
        }
        /// <summary>
        /// The name of the table that contains this field.
        /// </summary>
        field(3; TableName; Text[30])
        {
        }
        /// <summary>
        /// The name of the field as defined in the AL source code.
        /// </summary>
        field(4; FieldName; Text[30])
        {
        }
        /// <summary>
        /// The data type of the field (Text, Integer, Boolean, etc.).
        /// </summary>
        field(5; Type; Option)
        {
            OptionMembers = TableFilter,RecordID,OemText,Date,Time,DateFormula,Decimal,Media,MediaSet,Text,Code,Binary,BLOB,Boolean,Integer,OemCode,Option,BigInteger,Duration,GUID,DateTime;
            // This list must stay in sync with NCLOptionMetadataNavTypeField
            OptionOrdinalValues = 4912, 4988, 11519, 11775, 11776, 11797, 12799, 26207, 26208, 31488, 31489, 33791, 33793, 34047, 34559, 35071, 35583, 36095, 36863, 37119, 37375;
        }
        /// <summary>
        /// The length or size constraint for text and code fields.
        /// </summary>
        field(6; Len; Integer)
        {
        }
        /// <summary>
        /// The field class indicating whether it's a normal field, FlowField, or FlowFilter.
        /// </summary>
        field(7; Class; Option)
        {
            OptionMembers = Normal,FlowField,FlowFilter;
        }
        /// <summary>
        /// Indicates whether the field is currently enabled and accessible.
        /// </summary>
        field(8; Enabled; Boolean)
        {
        }
        /// <summary>
        /// The type name for complex field types such as enums or custom types.
        /// </summary>
        field(9; "Type Name"; Text[30])
        {
        }
        /// <summary>
        /// The external name used when the field is exposed to external systems or APIs.
        /// </summary>
        field(10; ExternalName; Text[100])
        {
        }
        /// <summary>
        /// The caption text displayed for this field in the user interface.
        /// </summary>
        field(20; "Field Caption"; Text[80])
        {
        }
        /// <summary>
        /// The table number of the related table when this field has a table relation.
        /// </summary>
        field(21; RelationTableNo; Integer)
        {
        }
        /// <summary>
        /// The field number in the related table when this field has a table relation.
        /// </summary>
        field(22; RelationFieldNo; Integer)
        {
        }
        /// <summary>
        /// The SQL data type used for this field in the underlying database.
        /// </summary>
        field(23; SQLDataType; option)
        {
            OptionMembers = Varchar,Integer,Variant,BigInteger;
        }
        /// <summary>
        /// The option string containing all possible values for option fields.
        /// </summary>
        field(24; OptionString; Text[2047])
        {
        }
        /// <summary>
        /// The obsolete state indicating whether the field is obsolete, pending removal, or removed.
        /// </summary>
        field(25; ObsoleteState; Option)
        {
            OptionMembers = No,Pending,Removed;
        }
        /// <summary>
        /// The reason provided when the field was marked as obsolete.
        /// </summary>
        field(26; ObsoleteReason; Text[248])
        {
        }
        /// <summary>
        /// The data classification category for privacy and compliance purposes.
        /// </summary>
        field(27; DataClassification; Option)
        {
            OptionMembers = CustomerContent,ToBeClassified,EndUserIdentifiableInformation,AccountData,EndUserPseudonymousIdentifiers,OrganizationIdentifiableInformation,SystemMetadata;
        }
        /// <summary>
        /// Indicates whether this field is part of the table's primary key.
        /// </summary>
        field(28; IsPartOfPrimaryKey; Boolean)
        {
        }
        /// <summary>
        /// The package identifier of the application that contains this field.
        /// </summary>
        field(60; "App Package ID"; Guid)
        {
        }
        /// <summary>
        /// The runtime package identifier used by the platform for application management.
        /// </summary>
        field(61; "App Runtime Package ID"; Guid)
        {
        }
        /// <summary>
        /// Indicates whether the field is optimized for text search operations.
        /// </summary>
        field(62; "OptimizeForTextSearch"; Boolean)
        {
        }
        /// <summary>
        /// The access level of the field (Public, Internal, Protected, or Local).
        /// </summary>
        field(63; Access; Option)
        {
            OptionMembers = Public,Internal,Protected,Local;
        }
        /// <summary>
        /// Indicates whether the field can be modified through customization features.
        /// </summary>
        field(64; IsAllowedInCustomizations; Boolean)
        {
        }
    }

    keys
    {
        key(pk; TableNo, "No.")
        {
        }
    }
}
