// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Environment.Configuration;

/// <summary>
/// Stores user-specific and system-wide configuration options for Business Central objects such as reports, XMLports, and pages.
/// </summary>
/// <remarks>
/// This table manages configuration settings and options for various Business Central objects, enabling personalization
/// and customization of object behavior. Supports user-specific settings as well as company-wide configurations.
/// Options can be marked as public visible or temporary, and include metadata about creation and visibility.
/// Used by the object configuration framework to persist user preferences, report settings, and other object-specific
/// configuration data across sessions.
/// </remarks>
table 2000000196 "Object Options"
{
    Caption = 'Object Options';
    DataPerCompany = false;
    ReplicateData = false;
    Scope = Cloud;

    fields
    {
        /// <summary>
        /// Name of the configuration parameter being stored.
        /// </summary>
        field(1; "Parameter Name"; Text[50])
        {
            Caption = 'Parameter Name';
        }
        /// <summary>
        /// ID of the Business Central object this option applies to.
        /// </summary>
        field(2; "Object ID"; Integer)
        {
            Caption = 'Object ID';
        }
        /// <summary>
        /// Type of Business Central object (Report, XMLport, Page) this option applies to.
        /// </summary>
        field(3; "Object Type"; Option)
        {
            Caption = 'Object Type';
            OptionMembers = ,,,"Report",,,"XMLport",,"Page",,,,,,,,,,,;
            OptionCaption = ',,,Report,,,XMLport,,"Page",,,,,,,,,,,';
        }
        /// <summary>
        /// Company context for this option setting, if company-specific.
        /// </summary>
        field(4; "Company Name"; Text[30])
        {
            Caption = 'Company Name';
            TableRelation = System.Environment.Company.Name;
        }
        /// <summary>
        /// User name for user-specific option settings.
        /// </summary>
        field(5; "User Name"; Code[50])
        {
            Caption = 'User Name';
        }
        /// <summary>
        /// Configuration data stored as BLOB to accommodate various option types and formats.
        /// </summary>
        field(6; "Option Data"; BLOB)
        {
            Caption = 'Option Data';
            SubType = UserDefined;
        }
        /// <summary>
        /// Indicates whether this option setting is visible to other users.
        /// </summary>
        field(7; "Public Visible"; Boolean)
        {
            Caption = 'Public Visible';
        }
        /// <summary>
        /// Indicates whether this is a temporary option setting that should not be persisted long-term.
        /// </summary>
        field(8; "Temporary"; Boolean)
        {
            Caption = 'Temporary';
        }
        /// <summary>
        /// User name of the person who created this option setting.
        /// </summary>
        field(9; "Created By"; Code[50])
        {
            Caption = 'Created By';
        }
    }

    keys
    {
        key(Key1; "Parameter Name", "Object ID", "Object Type", "User Name", "Company Name")
        {
            Clustered = true;
        }
    }

    fieldgroups
    {
        fieldgroup(DropDown; "Parameter Name")
        {
        }
    }
}

