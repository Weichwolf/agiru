// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Integration;

/// <summary>
/// Stores Entity Data Model (EDM) type definitions for OData service metadata and schema management in web service integrations.
/// </summary>
/// <remarks>
/// This table manages OData EDM metadata containing type definitions, relationships, and schema information for web service endpoints. Provides structured metadata for external system integration and API documentation generation. Used by the OData service layer for dynamic metadata generation and API schema validation.
/// </remarks>
table 2000000179 "OData Edm Type"
{
    Caption = 'OData Edm Type';
    DataPerCompany = false;
    Scope = Cloud;

    fields
    {
        /// <summary>
        /// Unique key identifier for the EDM type definition used for metadata lookup and schema references.
        /// </summary>
        field(1; "Key"; Code[50])
        {
            Caption = 'Key';
        }
        /// <summary>
        /// Descriptive text explaining the purpose and usage of the EDM type definition.
        /// </summary>
        field(2; Description; Text[250])
        {
            Caption = 'Description';
        }
        /// <summary>
        /// XML content containing the complete EDM schema definition with type structures and metadata.
        /// </summary>
        field(10; "Edm Xml"; BLOB)
        {
            Caption = 'Edm Xml';
            SubType = UserDefined;
        }
    }

    keys
    {
        key(Key1; "Key")
        {
            Clustered = true;
        }
    }

    fieldgroups
    {
    }
}

