// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Tooling;

/// <summary>
/// Virtual table that provides detailed information about fields available on pages, including both table fields and page-specific fields.
/// </summary>
/// <remarks>
/// This table exposes metadata about fields that can be used on pages, including their types, captions,
/// scope, and relationship to underlying table fields. Essential for page designer tools, field discovery,
/// and dynamic page construction. The table distinguishes between table fields, page fields bound to tables,
/// and page fields bound to expressions, providing detailed categorization for each field type.
/// </remarks>
table 2000000171 "Page Table Field"
{
    DataPerCompany = false;
    Scope = OnPrem;

    fields
    {
        /// <summary>
        /// The ID of the page that this field information relates to.
        /// </summary>
        field(1; "Page ID"; Integer)
        {
        }
        /// <summary>
        /// The sequential index of the field within the page context.
        /// </summary>
        field(2; Index; Integer)
        {
        }
        /// <summary>
        /// The data type of the field (e.g., Text, Integer, Boolean, Date, etc.).
        /// </summary>
        field(3; Type; Option)
        {
            OptionMembers = TableFilter,RecordID,OemText,Date,Time,DateFormula,Decimal,Media,MediaSet,Text,Code,NotSupported_Binary,BLOB,Boolean,Integer,OemCode,Option,BigInteger,Duration,GUID,DateTime;
            // This list must stay in sync with NCLOptionMetadataNavTypeField
            OptionOrdinalValues = 4912, 4988, 11519, 11775, 11776, 11797, 12799, 26207, 26208, 31488, 31489, 33791, 33793, 34047, 34559, 35071, 35583, 36095, 36863, 37119, 37375;
        }
        /// <summary>
        /// The maximum length of the field data, applicable for text and code fields.
        /// </summary>
        field(4; Length; Integer)
        {
        }
        /// <summary>
        /// The display caption of the field as shown in the user interface.
        /// </summary>
        field(5; Caption; Text[80])
        {
        }
        /// <summary>
        /// The placement status of the field on the page (obsolete - use Scope field instead).
        /// </summary>
        field(6; Status; Option)
        {
            OptionMembers = New,Ready,Placed;
            ObsoleteState = Pending;
            ObsoleteReason = 'This is being removed in favor of the Scope field which provides more granular information.';
        }
        /// <summary>
        /// Indicates whether the field is a table field or a page field bound to a table field (obsolete - use FieldKind field instead).
        /// </summary>
        field(7; IsTableField; Boolean)
        {
            ObsoleteState = Pending;
            ObsoleteReason = 'This is being removed in favor of the FieldKind field which provides more granular information.';
        }
        /// <summary>
        /// Specifies the source and visibility scope of the field on the page.
        /// </summary>
        field(8; Scope; Option)
        {
            OptionMembers = TableFieldVisibleOnPage,TableFieldHiddenOnPage,TableFieldNotOnPage,TableExtensionFieldVisibleOnPage,TableExtensionFieldHiddenOnPage,TableExtensionFieldNotOnPage,PageFieldVisible,PageFieldHidden,PageExtensionFieldVisible,PageExtensionFieldHidden;
        }
        /// <summary>
        /// JSON-formatted information used to build the field's tooltip in the user interface.
        /// </summary>
        field(9; Tooltip; Text[2048])
        {
        }
        /// <summary>
        /// The category of field based on its binding (table field, page field bound to table, or page field bound to expression).
        /// </summary>
        field(10; FieldKind; Option)
        {
            OptionMembers = TableField,PageFieldBoundToTable,PageFieldBoundToExpression;
        }
        /// <summary>
        /// The name identifier of the field.
        /// </summary>
        field(11; Name; Text[256])
        {
        }
        /// <summary>
        /// The field ID for table fields or runtime control ID for page fields.
        /// </summary>
        field(12; "Field ID"; Integer)
        {
        }
        /// <summary>
        /// The number of the source table that this field belongs to.
        /// </summary>
        field(13; "Table No"; Integer)
        {
        }
        /// <summary>
        /// The descriptive text or tooltip for the field, derived from AL tooltip or table field tooltip.
        /// </summary>
        field(14; Description; Text[2048])
        {
        }
        /// <summary>
        /// The table field ID for table or table-bound fields (0 for page fields bound to expressions).
        /// </summary>
        field(15; "Table Field Id"; Integer)
        {
        }
    }

    keys
    {
        key(pk; "Page ID", Index)
        {
        }
    }

    fieldGroups
    {
        fieldgroup("Brick"; Type, Caption, Status, Scope)
        {
        }
    }
}
