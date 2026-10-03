// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------
namespace System.Utilities;

/// <summary>
/// Virtual table that provides an infinite sequence of integer numbers for use in loops, calculations, and data generation.
/// This table is commonly used when you need to iterate over a range of numbers or generate sequential data.
/// </summary>
/// <remarks>
/// The Integer table is a fundamental utility table that contains an endless sequence of integer values.
/// It's frequently used in AL code for looping operations, generating test data, creating sequences,
/// and performing bulk operations where you need to iterate a specific number of times.
/// The table is virtual and dynamically generates numbers as needed, making it memory-efficient
/// for large ranges. Common usage patterns include FOR loops, data initialization, and report generation.
/// </remarks>
table 2000000026 Integer
{
    DataPerCompany = False;
    Scope = Cloud;
    fields
    {
        /// <summary>
        /// The integer value in the sequence, starting from negative values and continuing through positive values.
        /// </summary>
        field(1; Number; Integer)
        {
        }
    }

    Keys
    {
        key(pk; Number)
        {
        }
    }
}
