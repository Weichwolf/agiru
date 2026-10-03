#pragma once

#include <array>
#include <string_view>
#include <utility>

namespace interface_fixture {

inline constexpr std::string_view kContract = R"(
namespace Microsoft;
interface "Default Contract"
{
    procedure Add(var Value: Integer);
    procedure Add(var Value: Integer; Delta: Integer)
    begin
        this.Add(Value);
        Value += Delta;
    end;
    procedure Empty(var Value: Integer)
    begin
    end;
    procedure DefaultFee(): Integer
    begin
        exit(13);
    end;
    procedure DefaultFee(Scale: Integer): Integer
    begin
        exit(this.DefaultFee() * Scale);
    end;
    procedure Named(Value: Integer) Result: Integer
    var
        Copy: Integer;
    begin
        Copy := Value;
        Result := Copy + this.DefaultFee();
    end;
    procedure LabelValue(): Text
    var
        CaptionLbl: Label 'default value';
    begin
        exit(CaptionLbl);
    end;
    procedure Zero() Result: Integer
    begin
    end;
    procedure Options(): Boolean
    var Choice: Option First,Second;
    begin
        Choice := Choice::Second;
        exit(Choice = Choice::Second);
    end;
})";

inline constexpr std::string_view kChild = R"(
namespace Microsoft;
interface "Extended Contract" extends "Default Contract"
{
    procedure ExtendedFee(Scale: Integer): Integer
    begin
        exit(this.DefaultFee(Scale) + 1);
    end;
})";

inline constexpr std::string_view kDefaultUnit = R"(
namespace Microsoft;
codeunit 50000 "Default Consumer" implements "Default Contract"
{
    procedure Add(var Value: Integer)
    begin
        Value += 1;
    end;
})";

inline constexpr std::string_view kOverrideUnit = R"(
namespace Microsoft;
codeunit 50001 "Override Consumer" implements "Extended Contract"
{
    procedure Add(var Value: Integer)
    begin
        Value += 2;
    end;
    procedure DefaultFee(): Integer
    begin
        exit(21);
    end;
})";

inline constexpr std::string_view kCaller = R"(
namespace Microsoft;
codeunit 50002 "Interface Caller"
{
    procedure Check(): Boolean
    var
        Unit: Codeunit "Default Consumer";
        Contract: Interface "Default Contract";
        Value: Integer;
    begin
        Contract := Unit;
        Contract.Empty(Value);
        Contract.Add(Value, 4);
        exit((Value = 5) and (Contract.DefaultFee(3) = 39));
    end;
})";

inline constexpr std::array kSources{std::pair{"DefaultContract.Interface.al", kContract},
                                     std::pair{"ExtendedContract.Interface.al", kChild},
                                     std::pair{"DefaultConsumer.Codeunit.al", kDefaultUnit},
                                     std::pair{"OverrideConsumer.Codeunit.al", kOverrideUnit},
                                     std::pair{"InterfaceCaller.Codeunit.al", kCaller}};

}
