// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Privacy;

using System.Security.AccessControl;

/// <summary>
/// Records individual user approvals or rejections of privacy notices for integration services.
/// This table maintains an audit trail of privacy consent decisions for compliance and legal requirements.
/// </summary>
/// <remarks>
/// This table stores user-specific approval records linked to Privacy Notice definitions.
/// Each record represents a user's decision (approved or rejected) for a specific privacy notice.
/// Data is not company-specific to maintain consistent privacy compliance across the platform.
/// Extensibility points include adding approval workflow events and custom approval logic.
/// Related tables: Privacy Notice for the notice definitions, User table for user references.
/// </remarks>
table 2000000238 "Privacy Notice Approval"
{
    Caption = 'Privacy Notice Approval';
    DataPerCompany = false;
    ReplicateData = false;
    Scope = Cloud;
    InherentEntitlements = RIMDX;
    InherentPermissions = rX;

    fields
    {
        /// <summary>
        /// Reference to the Privacy Notice that was approved or rejected, establishing the link to the specific privacy policy.
        /// </summary>
        field(1; ID; Code[50])
        {
            Caption = 'Privacy Notice ID';
            TableRelation = "Privacy Notice";
        }
        /// <summary>
        /// Security identifier of the user who made the approval decision, used for tracking individual consent.
        /// </summary>
        field(2; "User SID"; Guid)
        {
            Caption = 'User SID';
            TableRelation = User."User Security ID";
        }
        /// <summary>
        /// Security identifier of the user who processed or recorded the approval, may differ from the User SID in delegation scenarios.
        /// </summary>
        field(3; "Approver User SID"; Guid)
        {
            Caption = 'Approver User ID';
            TableRelation = User."User Security ID";
        }
        /// <summary>
        /// Boolean flag indicating whether the privacy notice was approved (true) or rejected (false) by the user.
        /// </summary>
        field(4; Approved; Boolean)
        {
            Caption = 'Approved';
        }
    }

    keys
    {
        key(Key1; Id, "User SID")
        {
            Clustered = true;
        }
    }
}
