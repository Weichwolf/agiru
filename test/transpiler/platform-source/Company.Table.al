// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Environment;

/// <summary>
/// Stores company definitions and settings for Business Central tenants.
/// Manages company creation, configuration, and identification across the platform.
/// </summary>
/// <remarks>
/// This table serves as the central registry for all companies within a tenant, supporting
/// multi-company operations, company-specific data isolation, and business profile management.
/// Integrates with tenant provisioning, user access control, and company-specific configuration systems.
/// </remarks>
table 2000000006 Company
{
    Caption = 'Company';
    DataPerCompany = false;
    Scope = Cloud;
    ReplicateData = false;

    fields
    {
        /// <summary>
        /// Unique identifier name of the company used for database partitioning and access control.
        /// </summary>
        field(1; Name; Text[30])
        {
            Caption = 'Name';
        }
        /// <summary>
        /// Indicates whether this is an evaluation company with limited functionality and data restrictions.
        /// </summary>
        field(2; "Evaluation Company"; Boolean)
        {
            Caption = 'Evaluation Company';
        }
        /// <summary>
        /// User-friendly display name of the company shown in interfaces and reports.
        /// </summary>
        field(3; "Display Name"; Text[250])
        {
            Caption = 'Display Name';
        }
        /// <summary>
        /// Global unique identifier for the company used in integration and external references.
        /// </summary>
        field(8000; Id; Guid)
        {
            Caption = 'Id';
        }
        /// <summary>
        /// Business profile identifier linking the company to external business classification systems.
        /// </summary>
        field(8005; "Business Profile Id"; Text[250])
        {
            Caption = 'Business Profile Id';
        }
    }

    keys
    {
        key(Key1; Name)
        {
            Clustered = true;
        }
    }

    fieldgroups
    {
        fieldgroup(DropDown; Name, "Display Name", "Evaluation Company")
        {
        }
    }
}
