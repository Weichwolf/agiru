// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Reflection;

/// <summary>
/// Virtual table that provides metadata information about all tables in the system.
/// This table enables introspection of table properties, settings, and characteristics for development and administration purposes.
/// </summary>
/// <remarks>
/// The Table Metadata table is a fundamental metadata source that provides detailed information about
/// table definitions, properties, and configuration settings. It includes information about table types,
/// data classification, permissions, and various table attributes. This table is essential for
/// development tools, metadata analyzers, and administrative utilities that need to understand
/// table structures and properties. The information covers both base application tables and
/// those added by extensions, providing a complete view of the data model.
/// </remarks>
table 2000000136 "Table Metadata"
{
    DataPerCompany = False;
    Scope = Cloud;
    InherentPermissions = rX;

    fields
    {
        /// <summary>
        /// The unique identifier number of the table.
        /// </summary>
        field(1; ID; Integer)
        {
        }
        /// <summary>
        /// The name of the table as defined in the AL source code.
        /// </summary>
        field(2; Name; Text[30])
        {
        }
        /// <summary>
        /// The caption text displayed for this table in the user interface.
        /// </summary>
        field(3; Caption; Text[80])
        {
        }
        /// <summary>
        /// Indicates whether the table stores data separately for each company or globally.
        /// </summary>
        field(4; DataPerCompany; Boolean)
        {
        }
        /// <summary>
        /// The page ID used for lookup operations on this table.
        /// </summary>
        field(5; LookupPageID; Integer)
        {
        }
        /// <summary>
        /// The page ID used for drill-down operations on this table.
        /// </summary>
        field(6; DrillDownPageId; Integer)
        {
        }
        /// <summary>
        /// The fields used to generate captions for records in this table.
        /// </summary>
        field(7; DataCaptionFields; Text[80])
        {
        }
        /// <summary>
        /// Indicates whether paste operations are valid for this table.
        /// </summary>
        field(8; PasteIsValid; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether this table is linked to an external object or system.
        /// </summary>
        field(9; LinkedObject; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether the table data is stored externally rather than in the main database.
        /// </summary>
        field(10; DataIsExternal; Boolean)
        {
        }
        /// <summary>
        /// The type of table indicating its purpose and storage characteristics.
        /// </summary>
        field(11; TableType; Option)
        {
            OptionMembers = Normal,CRM,ExternalSQL,Exchange,MicrosoftGraph,Query,Temporary;
        }
        /// <summary>
        /// The external name used when the table is accessed from external systems.
        /// </summary>
        field(12; ExternalName; Text[248])
        {
        }
        /// <summary>
        /// The obsolete state indicating whether the table is obsolete, pending removal, or removed.
        /// </summary>
        field(13; ObsoleteState; Option)
        {
            OptionMembers = No,Pending,Removed;
        }
        /// <summary>
        /// The reason provided when the table was marked as obsolete.
        /// </summary>
        field(14; ObsoleteReason; Text[248])
        {
        }
        /// <summary>
        /// The data classification category for privacy and compliance purposes.
        /// </summary>
        field(15; DataClassification; Option)
        {
            OptionMembers = CustomerContent,ToBeClassified,EndUserIdentifiableInformation,AccountData,EndUserPseudonymousIdentifiers,OrganizationIdentifiableInformation,SystemMetadata;
        }
        /// <summary>
        /// Indicates whether data in this table should be replicated to other environments.
        /// </summary>
        field(16; ReplicateData; Boolean)
        {
            Caption = 'ReplicateData';
        }
        /// <summary>
        /// The compression type applied to the table for storage optimization.
        /// </summary>
        field(17; CompressionType; Option)
        {
            OptionMembers = Unspecified,None,Row,Page;
        }
        /// <summary>
        /// The application identifier that contains this table.
        /// </summary>
        field(18; "App ID"; Guid)
        {
        }
        /// <summary>
        /// The inherent permissions defined for this table (Read, Insert, Modify, Delete).
        /// </summary>
        field(19; InherentPermissions; Text[5])
        {
        }
        /// <summary>
        /// The inherent entitlements required to access this table.
        /// </summary>
        field(20; InherentEntitlements; Text[5])
        {
        }
        /// <summary>
        /// The deployment scope where this table is available (Cloud or OnPrem).
        /// </summary>
        field(21; Scope; Option)
        {
            OptionMembers = Cloud,OnPrem;
        }
        /// <summary>
        /// The access level of the table (Public or Internal).
        /// </summary>
        field(22; Access; Option)
        {
            OptionMembers = Public,Internal;
        }
        /// <summary>
        /// The AL namespace of the object.
        /// </summary>
        field(23; "AL Namespace"; text[500])
        {
        }
    }

    fieldgroups
    {
        fieldgroup("DropDown"; ID, Caption)
        {
        }
    }
}
