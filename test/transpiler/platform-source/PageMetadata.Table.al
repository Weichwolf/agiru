// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Reflection;

/// <summary>
/// Virtual table that provides metadata information about all pages in the system.
/// This table enables introspection of page properties, behavior settings, and configuration for development and administration.
/// </summary>
/// <remarks>
/// The Page Metadata table is a critical metadata source for understanding page definitions and their characteristics.
/// It includes detailed information about page types, source tables, permissions, API configurations, and various
/// behavioral properties. This table is essential for development tools, page analyzers, API management, and
/// administrative utilities that need to understand page structures and capabilities. The information covers
/// all page types including Cards, Lists, RoleCenters, APIs, and parts, providing a view of
/// the user interface and API surface area across the entire application suite.
/// </remarks>
table 2000000138 "Page Metadata"
{
    DataPerCompany = False;
    Scope = Cloud;
    InherentPermissions = rX;

    fields
    {
        /// <summary>
        /// The unique identifier number of the page.
        /// </summary>
        field(1; ID; Integer)
        {
        }
        /// <summary>
        /// The name of the page as defined in the AL source code.
        /// </summary>
        field(2; Name; Text[30])
        {
        }
        /// <summary>
        /// The caption text displayed for this page in the user interface.
        /// </summary>
        field(3; Caption; Text[80])
        {
        }
        /// <summary>
        /// Indicates whether the page allows editing of data.
        /// </summary>
        field(4; Editable; Boolean)
        {
        }
        /// <summary>
        /// The type of page defining its layout and behavior characteristics.
        /// </summary>
        field(5; PageType; Option)
        {
            OptionMembers = Card,List,RoleCenter,CardPart,ListPart,Document,Worksheet,ListPlus,ConfirmationDialog,NavigatePage,StandardDialog,API,HeadlinePart;
        }
        /// <summary>
        /// The page ID used for card operations when this is a list page.
        /// </summary>
        field(6; CardPageID; Integer)
        {
        }
        /// <summary>
        /// The expression used to generate the data caption for records displayed on this page.
        /// </summary>
        field(7; "DataCaptionExpr."; Text[250])
        {
        }
        /// <summary>
        /// Indicates whether the page should refresh its data when it becomes active.
        /// </summary>
        field(8; RefreshOnActivate; Boolean)
        {
        }
        /// <summary>
        /// The publisher name for API pages when exposed through web services.
        /// </summary>
        field(9; APIPublisher; Text[40])
        {
        }
        /// <summary>
        /// The API group classification for API pages.
        /// </summary>
        field(10; APIGroup; Text[40])
        {
        }
        /// <summary>
        /// The API version identifier for API pages.
        /// </summary>
        field(11; APIVersion; Text[250])
        {
        }
        /// <summary>
        /// The entity set name used in OData APIs for this page.
        /// </summary>
        field(12; EntitySetName; Text[250])
        {
        }
        /// <summary>
        /// The entity name used in OData APIs for this page.
        /// </summary>
        field(13; EntityName; Text[250])
        {
        }
        /// <summary>
        /// The table number that serves as the data source for this page.
        /// </summary>
        field(14; SourceTable; Integer)
        {
        }
        /// <summary>
        /// The default table view (sorting and filtering) applied to the source table.
        /// </summary>
        field(15; SourceTableView; Text[250])
        {
        }
        /// <summary>
        /// Indicates whether new records can be inserted through this page.
        /// </summary>
        field(16; InsertAllowed; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether existing records can be modified through this page.
        /// </summary>
        field(17; ModifyAllowed; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether records can be deleted through this page.
        /// </summary>
        field(18; DeleteAllowed; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether insert operations are delayed until the user moves to another record.
        /// </summary>
        field(19; DelayedInsert; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether the filter pane is shown by default on this page.
        /// </summary>
        field(20; ShowFilter; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether multiple new lines can be created simultaneously on this page.
        /// </summary>
        field(21; MultipleNewLines; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether field values are automatically saved when navigating between records.
        /// </summary>
        field(22; SaveValues; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether primary key fields are automatically incremented for new records.
        /// </summary>
        field(23; AutoSplitKey; Boolean)
        {
        }
        /// <summary>
        /// The fields used to generate captions for records displayed on this page.
        /// </summary>
        field(24; DataCaptionFields; Text[250])
        {
        }
        /// <summary>
        /// Indicates whether the source table is used as a temporary table for this page.
        /// </summary>
        field(25; SourceTableTemporary; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether hyperlinks and drill-down operations are allowed on this page.
        /// </summary>
        field(26; LinksAllowed; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether change tracking is enabled for data modifications on this page.
        /// </summary>
        field(27; ChangeTrackingAllowed; Boolean)
        {
        }
        /// <summary>
        /// Indicates whether all fields should be populated when creating new records.
        /// </summary>
        field(28; PopulateAllFields; Boolean)
        {
        }
        /// <summary>
        /// The application identifier that contains this page.
        /// </summary>
        field(29; "App ID"; Guid)
        {
        }
        /// <summary>
        /// The inherent permissions defined for this page (Read, Insert, Modify, Delete).
        /// </summary>
        field(30; InherentPermissions; Text[5])
        {
        }
        /// <summary>
        /// The inherent entitlements required to access this page.
        /// </summary>
        field(31; InherentEntitlements; Text[5])
        {
        }
        /// <summary>
        /// The AL namespace of the object.
        /// </summary>
        field(32; "AL Namespace"; text[500])
        {
        }
    }

    keys
    {
        key(pk; ID)
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
