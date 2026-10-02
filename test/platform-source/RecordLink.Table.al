// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Environment.Configuration;

/// <summary>
/// Stores hyperlinks and external references associated with Business Central records.
/// Enables users to attach URLs, documents, and external resources to any record in the system.
/// </summary>
/// <remarks>
/// This table provides record-level link management functionality, allowing users to create
/// associations between records and external content. Integrates with note and attachment systems,
/// user interface link displays, and record navigation features for enhanced user productivity.
/// </remarks>
table 2000000068 "Record Link"
{
    Caption = 'Record Link';
    DataPerCompany = false;
    ReplicateData = false;
    Scope = Cloud;
    InherentPermissions = rX;

    fields
    {
        /// <summary>
        /// Auto-incrementing unique identifier for each record link, providing primary key functionality.
        /// </summary>
        field(1; "Link ID"; Integer)
        {
            AutoIncrement = true;
            Caption = 'Link ID';
        }
        /// <summary>
        /// RecordID that uniquely identifies the Business Central record this link is associated with.
        /// </summary>
        field(2; "Record ID"; RecordID)
        {
            Caption = 'Record ID';
        }
        /// <summary>
        /// The URL or hyperlink address pointing to the external resource or document.
        /// </summary>
        field(3; URL1; Text[2048])
        {
            Caption = 'URL1';
        }
        /// <summary>
        /// Obsolete URL field, replaced by expanded URL1 field for better link storage capacity.
        /// </summary>
        field(4; URL2; Text[250])
        {
            Caption = 'URL2';
            ObsoleteReason = 'URL1 field size increased';
            ObsoleteState = Removed;
        }
        /// <summary>
        /// Obsolete URL field, replaced by expanded URL1 field for better link storage capacity.
        /// </summary>
        field(5; URL3; Text[250])
        {
            Caption = 'URL3';
            ObsoleteReason = 'URL1 field size increased';
            ObsoleteState = Removed;
        }
        /// <summary>
        /// Obsolete URL field, replaced by expanded URL1 field for better link storage capacity.
        /// </summary>
        field(6; URL4; Text[250])
        {
            Caption = 'URL4';
            ObsoleteReason = 'URL1 field size increased';
            ObsoleteState = Removed;
        }
        /// <summary>
        /// Human-readable description of the link or note, providing context for users about the linked content.
        /// </summary>
        field(7; Description; Text[250])
        {
            Caption = 'Description';
        }
        /// <summary>
        /// Specifies whether this record link is a hyperlink to external content or an internal note attachment.
        /// </summary>
        field(8; Type; Option)
        {
            Caption = 'Type';
            OptionCaption = 'Link,Note';
            OptionMembers = Link,Note;
        }
        /// <summary>
        /// Binary data containing note text when the record link type is set to Note rather than Link.
        /// </summary>
        field(9; Note; BLOB)
        {
            Caption = 'Note';
            SubType = Memo;
        }
        /// <summary>
        /// Date and time when this record link was created, used for tracking and audit purposes.
        /// </summary>
        field(10; Created; DateTime)
        {
            Caption = 'Created';
        }
        /// <summary>
        /// Identifier of the user who created this record link, used for ownership and permission management.
        /// </summary>
        field(11; "User ID"; Text[132])
        {
            Caption = 'User ID';
        }
        /// <summary>
        /// Name of the company context in which this record link was created, enabling company-specific link management.
        /// </summary>
        field(12; Company; Text[30])
        {
            Caption = 'Company';
            TableRelation = System.Environment.Company.Name;
        }
        /// <summary>
        /// Indicates whether notifications should be sent regarding this record link, enabling collaborative link sharing.
        /// </summary>
        field(13; Notify; Boolean)
        {
            Caption = 'Notify';
        }
        /// <summary>
        /// Identifier of the target user who should receive notifications about this record link, supporting directed communication.
        /// </summary>
        field(14; "To User ID"; Text[132])
        {
            Caption = 'To User ID';
        }
    }

    keys
    {
        key(Key1; "Link ID")
        {
            Clustered = true;
        }
        key(Key2; "Record ID")
        {
        }
        key(Key3; Company, "Record ID")
        {
        }
    }

    fieldgroups
    {
    }
}
