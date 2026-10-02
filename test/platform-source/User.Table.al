// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Security.AccessControl;

/// <summary>
/// Stores user account information and security settings for all users in the Business Central tenant.
/// Manages user authentication, licensing, and access control across companies and applications.
/// </summary>
/// <remarks>
/// This table serves as the central user repository for security management, license assignment,
/// and user authentication. Integrates with Azure Active Directory, permission sets, and user groups.
/// Used by authentication services, user management interfaces, and security audit processes.
/// </remarks>
table 2000000120 User
{
    Caption = 'User';
    DataPerCompany = false;
    ReplicateData = false;
    Scope = Cloud;
    CompressionType = None;

    fields
    {
        /// <summary>
        /// Unique global identifier for the user account in the Business Central security system.
        /// </summary>
        field(1; "User Security ID"; Guid)
        {
            Caption = 'User Security ID';
        }
        /// <summary>
        /// Login name used for user authentication and identification.
        /// </summary>
        field(2; "User Name"; Code[50])
        {
            Caption = 'User Name';
        }
        /// <summary>
        /// Complete display name of the user for identification and user interface purposes.
        /// </summary>
        field(3; "Full Name"; Text[80])
        {
            Caption = 'Full Name';
        }
        /// <summary>
        /// Current activation status of the user account (Enabled or Disabled).
        /// </summary>
        field(4; State; Option)
        {
            Caption = 'State';
            OptionCaption = 'Enabled,Disabled';
            OptionMembers = Enabled,Disabled;
        }
        /// <summary>
        /// Date and time when the user account expires and access is automatically revoked.
        /// </summary>
        field(5; "Expiry Date"; DateTime)
        {
            Caption = 'Expiry Date';
        }
        /// <summary>
        /// Windows Active Directory security identifier for domain-authenticated users.
        /// </summary>
        field(7; "Windows Security ID"; Text[119])
        {
            Caption = 'Windows Security ID';
        }
        /// <summary>
        /// Indicates whether the user must change their password on next login.
        /// </summary>
        field(8; "Change Password"; Boolean)
        {
            Caption = 'Change Password';
        }
        /// <summary>
        /// License type assigned to the user determining available functionality and restrictions.
        /// </summary>
        field(10; "License Type"; Option)
        {
            Caption = 'License Type';
            OptionCaption = 'Full User,Limited User,Device Only User,Windows Group,External User,External Administrator,External Accountant,Application,AAD Group,Agent';
            OptionMembers = "Full User","Limited User","Device Only User","Windows Group","External User","External Administrator","External Accountant","Application","AAD Group","Agent";
        }
        /// <summary>
        /// Email address used for authentication and Azure Active Directory integration.
        /// </summary>
        field(11; "Authentication Email"; Text[250])
        {
            Caption = 'Authentication Email';
        }
        /// <summary>
        /// Primary contact email address for communications and notifications.
        /// </summary>
        field(14; "Contact Email"; Text[250])
        {
            Caption = 'Contact Email';
        }
        /// <summary>
        /// Exchange server identifier for Microsoft Office and email integration.
        /// </summary>
        field(15; "Exchange Identifier"; Text[250])
        {
            Caption = 'Exchange Identifier';
        }
        /// <summary>
        /// Azure Active Directory application identifier for service principal accounts.
        /// </summary>
        field(16; "Application ID"; Guid)
        {
            Caption = 'Application ID';
        }
    }

    keys
    {
        key(Key1; "User Security ID")
        {
            Clustered = true;
        }
        key(Key2; "User Name")
        {
        }
        key(Key3; "Windows Security ID")
        {
        }
    }

    fieldgroups
    {
        fieldgroup(DropDown; "User Name", "Full Name")
        {
        }
    }
}
