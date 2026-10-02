// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Privacy;

using System.Security.AccessControl;

/// <summary>
/// Manages privacy notices for integration services, enabling users to view and approve privacy terms before using external services.
/// This table provides a centralized mechanism for privacy compliance and user consent management.
/// </summary>
/// <remarks>
/// This table stores privacy notice definitions for various integration services and tracks user approval status through FlowFields.
/// Data is not company-specific to ensure consistent privacy notice management across the platform.
/// Extensibility points include adding custom privacy notice types and integration patterns through events.
/// Related tables: Privacy Notice Approval for tracking individual user approvals.
/// </remarks>
table 2000000237 "Privacy Notice"
{
    Caption = 'Privacy Notice';
    DataPerCompany = false;
    ReplicateData = false;
    Scope = Cloud;

    fields
    {
        /// <summary>
        /// Unique identifier for the privacy notice, used to link approvals and manage different privacy policies.
        /// </summary>
        field(1; ID; Code[50])
        {
            Caption = 'Privacy Notice ID';
        }
        /// <summary>
        /// Name of the integration service that requires privacy notice approval, displayed to users for identification.
        /// </summary>
        field(2; "Integration Service Name"; Text[250])
        {
            Caption = 'Integration Service Name';
        }
        /// <summary>
        /// URL link to the detailed privacy policy document that users can access for complete terms and conditions.
        /// </summary>
        field(3; Link; Text[2048])
        {
            Caption = 'Privacy Link';
        }
        /// <summary>
        /// Filter field for querying approval status for a specific user, used by FlowFields to calculate user-specific approval state.
        /// </summary>
        field(4; "User SID Filter"; Guid)
        {
            Caption = 'User SID Filter';
            TableRelation = User."User Security ID";
            FieldClass = FlowFilter;
        }
        /// <summary>
        /// Indicates whether the privacy notice has been approved by the user specified in the User SID Filter. Calculated from Privacy Notice Approval records.
        /// </summary>
        field(5; Enabled; Boolean)
        {
            // Note: In case both Enabled and Disabled are false, it means no decision has been made by the user in the User SID Filter.
            Caption = 'Enabled';
            FieldClass = FlowField;
            CalcFormula = exist("Privacy Notice Approval" where(ID = field(ID), "User SID" = field("User SID Filter"), Approved = const(true)));
        }
        /// <summary>
        /// Indicates whether the privacy notice has been explicitly rejected by the user specified in the User SID Filter. Calculated from Privacy Notice Approval records.
        /// </summary>
        field(6; Disabled; Boolean)
        {
            // Note: In case both Enabled and Disabled are false, it means no decision has been made by the user in the User SID Filter.
            Caption = 'Disabled';
            FieldClass = FlowField;
            CalcFormula = exist("Privacy Notice Approval" where(ID = field(ID), "User SID" = field("User SID Filter"), Approved = const(false)));
        }
    }

    keys
    {
        key(Key1; Id)
        {
            Clustered = true;
        }
        key(Key2; "Integration Service Name")
        {
        }
    }

    fieldgroups
    {
        fieldgroup(Brick; "Integration Service Name")
        {
        }
    }
}
