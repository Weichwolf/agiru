// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Reflection;

/// <summary>
/// Virtual table that provides information about all user profiles available in the system.
/// This table enables profile discovery, management, and configuration across all applications and scopes.
/// </summary>
/// <remarks>
/// The All Profile table is central to Business Central's role-based user interface system, providing
/// complete visibility into all available user profiles including system profiles and tenant-specific
/// profiles. It includes information about role centers, personalization settings, enablement status,
/// and various profile characteristics. This table is essential for user management, profile assignment,
/// and administrative tasks related to user experience customization. The table supports both system-wide
/// profiles and tenant-specific profiles, enabling flexible role-based access and interface customization
/// across different organizational structures and applications.
/// </remarks>
table 2000000178 "All Profile"
{
    Caption = 'All Profile';
    DataPerCompany = false;
    Scope = Cloud;
    InherentPermissions = rX;

    fields
    {
        /// <summary>
        /// The scope of the profile (System-wide or Tenant-specific).
        /// </summary>
        field(1; Scope; Option)
        {
            OptionMembers = System,Tenant;
            OptionCaption = 'System,Tenant';
        }
        /// <summary>
        /// The application identifier that contains this profile.
        /// </summary>
        field(2; "App ID"; Guid)
        {
        }
        /// <summary>
        /// The unique identifier of the profile.
        /// </summary>
        field(3; "Profile ID"; Code[30])
        {
        }
        /// <summary>
        /// The detailed description of the profile and its intended use.
        /// </summary>
        field(4; Description; Text[2048])
        {
        }
        /// <summary>
        /// The page ID of the Role Center associated with this profile.
        /// </summary>
        field(5; "Role Center ID"; Integer)
        {
        }
        /// <summary>
        /// Indicates whether this profile uses the default Role Center configuration.
        /// </summary>
        field(6; "Default Role Center"; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether the profile supports comments functionality (obsolete).
        /// </summary>
        field(7; "Use Comments"; Boolean)
        {
            ObsoleteState = Pending;
            ObsoleteReason = 'Capacity related to System profiles for which support has been removed.';
        }
        /// <summary>
        /// Indicates whether the profile supports notes functionality (obsolete).
        /// </summary>
        field(8; "Use Notes"; Boolean)
        {
            ObsoleteState = Pending;
            ObsoleteReason = 'Capacity related to System profiles for which support has been removed.';
        }
        /// <summary>
        /// Indicates whether the profile supports record-specific notes (obsolete).
        /// </summary>
        field(9; "Use Record Notes"; Boolean)
        {
            ObsoleteState = Pending;
            ObsoleteReason = 'Capacity related to System profiles for which support has been removed.';
        }
        /// <summary>
        /// The record notebook configuration for the profile (obsolete).
        /// </summary>
        field(10; "Record Notebook"; Text[250])
        {
            ObsoleteState = Pending;
            ObsoleteReason = 'Capacity related to System profiles for which support has been removed.';
        }
        /// <summary>
        /// Indicates whether the profile supports page-specific notes (obsolete).
        /// </summary>
        field(11; "Use Page Notes"; Boolean)
        {
            ObsoleteState = Pending;
            ObsoleteReason = 'Capacity related to System profiles for which support has been removed.';
        }
        /// <summary>
        /// The page notebook configuration for the profile (obsolete).
        /// </summary>
        field(12; "Page Notebook"; Text[250])
        {
            ObsoleteState = Pending;
            ObsoleteReason = 'Capacity related to System profiles for which support has been removed.';
        }
        /// <summary>
        /// Indicates whether personalization features are disabled for users of this profile.
        /// </summary>
        field(13; "Disable Personalization"; Boolean)
        {
        }
        /// <summary>
        /// The name of the application that contains this profile.
        /// </summary>
        field(14; "App Name"; Text[250])
        {
        }
        /// <summary>
        /// Indicates whether the profile is currently enabled and available for assignment to users.
        /// </summary>
        field(15; "Enabled"; Boolean)
        {
        }
        /// <summary>
        /// The caption text displayed for this profile in user interfaces.
        /// </summary>
        field(16; Caption; Text[100])
        {
        }
        /// <summary>
        /// Indicates whether the profile is promoted and highlighted in profile selection interfaces.
        /// </summary>
        field(17; Promoted; Boolean)
        {
        }
    }
    keys
    {
        key(PK; Scope, "App ID", "Profile ID")
        {
            Clustered = true;
        }
    }
    fieldgroups
    {
        fieldgroup(DropDown; "Profile ID", "App Name")
        {
        }
        fieldgroup(Brick; "App Name", Caption, "Profile ID")
        {
        }
    }
}
