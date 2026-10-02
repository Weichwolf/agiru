#pragma once

#include <string_view>

namespace report_layout_fixture {

inline constexpr std::string_view kReport = R"(namespace Microsoft.Test.Reporting;
report 50080 "Layout Contract"
{
    DefaultRenderingLayout = "Original Layout";
    UseRequestPage = false;
    dataset { }
    requestpage { }
    rendering
    {
        layout("Original Layout")
        {
            Type = Word;
            LayoutFile = 'Layouts\Original.docx';
            Caption = 'Writer''s original', Comment = 'Keep localization metadata';
            Summary = 'A layout, not a dataset dump';
            FutureProperty = 'Keep unknown declarations visible';
        }
    }
})";

inline constexpr std::string_view kExtension = R"(namespace Microsoft.Test.Reporting;
reportextension 50081 "Extra Layouts" extends "Layout Contract"
{
    rendering
    {
        layout("Theme Part")
        {
            Type = Word;
            Subtype = Theme;
            LayoutFile = 'Layouts\Theme.dotx';
            Caption = 'Theme';
        }
        layout("Spreadsheet")
        {
            Type = Excel;
            LayoutFile = 'Layouts/Spreadsheet.xlsx';
        }
    }
})";

inline constexpr std::string_view kUnresolved = R"(namespace Microsoft.Test.Reporting;
reportextension 50082 "Unresolved Layout" extends "Absent Native Report"
{
    rendering
    {
        layout("Unresolved")
        {
            Type = Custom;
            MimeType = 'application/x-agiru-layout-test';
            LayoutFile = 'Layouts/External.layout';
        }
    }
})";

}
