// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Utilities;

/// <summary>
/// Virtual table that provides period-based date calculations and date range generation for various time periods.
/// This table enables easy creation of date ranges for weeks, months, quarters, and years.
/// </summary>
/// <remarks>
/// The Date table is a utility table that automatically generates date periods and ranges
/// for reporting, filtering, and date-based calculations. It provides convenient access to
/// predefined date periods without requiring manual date arithmetic. This table is commonly used
/// in reports, charts, and analytics where data needs to be grouped or filtered by time periods.
/// The table dynamically calculates period boundaries and provides both localized and invariant names
/// for international applications.
/// </remarks>
table 2000000007 "Date"
{
    DataPerCompany = False;
    Scope = Cloud;
    InherentPermissions = RX;

    fields
    {
        /// <summary>
        /// The type of date period (Date, Week, Month, Quarter, or Year).
        /// </summary>
        field(1; "Period Type"; Option)
        {
            OptionMembers = "Date",Week,Month,Quarter,Year;
        }
        /// <summary>
        /// The start date of the period.
        /// </summary>
        field(2; "Period Start"; Date)
        {
        }
        /// <summary>
        /// The end date of the period.
        /// </summary>
        field(3; "Period End"; Date)
        {
        }
        /// <summary>
        /// The sequential number of the period within its type (e.g., week number, quarter number).
        /// </summary>
        field(4; "Period No."; Integer)
        {
        }
        /// <summary>
        /// The localized name of the period formatted according to the user's language and region settings.
        /// </summary>
        field(5; "Period Name"; Text[31])
        {
        }
        /// <summary>
        /// The culture-invariant name of the period that remains consistent across different locales.
        /// </summary>
        field(6; "Period Invariant Name"; Text[31])
        {
        }
    }

    keys
    {
        key(pk; "Period Type", "Period Start")
        {

        }
    }
}
