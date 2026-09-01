#include "Ast.h"
#include "Check.h"
#include "Lexer.h"
#include "Parser.h"
#include "Token.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using agiru::al::Find;
using agiru::al::ListValue;
using agiru::al::ParseTable;
using agiru::al::TableObject;

namespace {

constexpr std::string_view kSource = R"AL(
// ------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// ------------------------------------------------------------------------------------------------
namespace Microsoft.Projects.Resources.Pricing;

using Microsoft.Projects.Resources.Resource;

table 202 "Resource Cost"
{
    Caption = 'Resource Cost';
    DataClassification = CustomerContent;

    fields
    {
        field(1; Type; Option)
        {
            Caption = 'Type';
            OptionCaption = 'Resource,Group(Resource),All';
            OptionMembers = Resource,"Group(Resource)",All;
        }
        field(2; "Code"; Code[20])
        {
            Caption = 'Code';
            TableRelation = if (Type = const(Resource)) Resource
            else
            if (Type = const("Group(Resource)")) "Resource Group";

            trigger OnValidate()
            begin
                if (Code <> '') and (Type = Type::All) then
                    FieldError(Code, StrSubstNo(Text000, FieldCaption(Type), Format(Type)));
            end;
        }
        field(3; "Work Type Code"; Code[10])
        {
            Caption = 'Work Type Code';
            TableRelation = "Work Type";
        }
        field(6; "Unit Cost"; Decimal)
        {
            AutoFormatType = 2;
            Caption = 'Unit Cost';
        }
    }

    keys
    {
        key(Key1; Type, "Code", "Work Type Code")
        {
            Clustered = true;
        }
        key(Key2; "Cost Type", "Code", "Work Type Code")
        {
        }
    }

    fieldgroups
    {
    }

    var
#pragma warning disable AA0074
        Text000: Label 'cannot be specified when %1 is %2';
#pragma warning restore AA0074
}
)AL";

void TheObjectHeaderIsRead(const TableObject &table) {
  CHECK_TRUE("the table number", table.id == 202);
  CHECK_TEXT("the AL name, unquoted", table.name, "Resource Cost");
  CHECK_TEXT("an object property", Find(table.properties, "Caption")->text, "Resource Cost");
  // AL IS CASE-INSENSITIVE, so a property is found however it was spelled. Diverging casing
  // produced two symbols in the predecessor and cost it a measured failure class.
  CHECK_TRUE("a property is found case-insensitively",
             Find(table.properties, "cAPTION") != nullptr);
}

void EveryFieldCarriesItsDeclaration(const TableObject &table) {
  CHECK_TRUE("four fields", table.fields.size() == 4);
  CHECK_TRUE("the numbers are as declared, gaps and all",
             table.fields[0].number == 1 && table.fields[3].number == 6);
  CHECK_TEXT("a quoted name loses its quotes", table.fields[2].name, "Work Type Code");
  CHECK_TEXT("an unquoted name is kept", table.fields[0].name, "Type");
  CHECK_TEXT("the type", table.fields[1].type, "Code");
  CHECK_TRUE("the declared length", table.fields[1].length == 20);
  CHECK_TRUE("a type without a length has none", table.fields[3].length == 0);
  CHECK_TEXT(
      "a field property", Find(table.fields[2].properties, "Caption")->text, "Work Type Code");
}

void AnOptionCarriesItsMembers(const TableObject &table) {
  const auto *members = Find(table.fields[0].properties, "OptionMembers");
  CHECK_TRUE("OptionMembers is read", members != nullptr);
  const std::vector<std::string> names = ListValue(*members);
  CHECK_TRUE("three members", names.size() == 3);
  // The member that cannot be a C++ identifier keeps its AL spelling here, which is what an error
  // message and a filter string have to say.
  CHECK_TEXT("a member that is no identifier", names[1], "Group(Resource)");
  CHECK_TEXT("and the last one", names[2], "All");
}

void ATriggerIsCapturedWhole(const TableObject &table) {
  const auto *trigger = Find(table.fields[1].triggers, "OnValidate");
  CHECK_TRUE("the field's OnValidate is found", trigger != nullptr);
  CHECK_TRUE("its body carries the call", std::ranges::any_of(trigger->tokens, [](const auto &t) {
               return t.text == "FieldError";
             }));
  // An empty string literal is a TOKEN, not nothing: `Code <> ''` compares against the empty
  // string, and a body kept as text would have lost the difference between '' and absent.
  CHECK_TRUE("an empty string literal is a token of its own",
             std::ranges::any_of(trigger->tokens, [](const auto &t) {
               return t.kind == agiru::al::TokenKind::String && t.text.empty();
             }));
  CHECK_TRUE("a field without a trigger has none", table.fields[0].triggers.empty());
}

void TheKeysAreRead(const TableObject &table) {
  CHECK_TRUE("two keys", table.keys.size() == 2);
  CHECK_TEXT("the key name", table.keys[0].name, "Key1");
  CHECK_TRUE("three key fields", table.keys[0].fields.size() == 3);
  CHECK_TEXT("a quoted key field loses its quotes", table.keys[0].fields[2], "Work Type Code");
  CHECK_TEXT("Clustered", Find(table.keys[0].properties, "Clustered")->text, "true");
  CHECK_TRUE("a key without properties has none", table.keys[1].properties.empty());
}

void TheLabelsAreRead(const TableObject &table) {
  CHECK_TRUE("one label", table.labels.size() == 1);
  CHECK_TEXT("its name", table.labels[0].name, "Text000");
  CHECK_TEXT("its text", table.labels[0].text, "cannot be specified when %1 is %2");
}

/// The same parse, over the file the target image was written from. A parser that only reads the
/// excerpt in this case is a parser that has been fitted to its test.
void TheRealFileParses() {
  const std::filesystem::path path =
      std::filesystem::path(AGIRU_AL_SOURCE) / "Projects/Resources/Pricing/ResourceCost.Table.al";
  if (!std::filesystem::exists(path)) {
    CHECK_TRUE("the AL source is where CLAUDE.md says it is", false);
    return;
  }
  const std::ifstream file(path);
  std::ostringstream text;
  text << file.rdbuf();
  const TableObject table = ParseTable(text.str());
  CHECK_TRUE("table 202 out of the real file", table.id == 202);
  CHECK_TEXT("with its name", table.name, "Resource Cost");
  CHECK_TRUE("six fields", table.fields.size() == 6);
  CHECK_TRUE("two keys", table.keys.size() == 2);
  CHECK_TRUE("one label", table.labels.size() == 1);
  CHECK_TRUE("both OnValidate triggers",
             Find(table.fields[1].triggers, "OnValidate") != nullptr &&
                 Find(table.fields[3].triggers, "OnValidate") != nullptr);
}

/// WHAT THE LEXER MUST ACCOUNT FOR, and every line here is a lesson the predecessor paid for
/// (openerp `scripts/transpiler/parser/lexer.py`). None of it is in the platform documentation:
/// it is what real BaseApp source contains.
void TheLexerReadsWhatBaseAppActuallyContains() {
  using agiru::al::Tokenize;
  using agiru::al::TokenKind;

  // A quoted identifier escapes a quote as "". Without this the token ends early and the rest of
  // the line fragments into stray literals -- openerp hit it on TestPage field chains.
  const auto quoted = Tokenize(R"(x := "ConsiderSource[SourceType::""Liquid Funds""]";)");
  CHECK_TRUE("a doubled quote inside a quoted identifier stays inside it",
             quoted[2].kind == TokenKind::QuotedIdentifier);
  CHECK_TEXT("and the identifier is one token",
             quoted[2].text,
             R"(ConsiderSource[SourceType::"Liquid Funds"])");

  // AL date and time literals carry a letter suffix. Lexed as a number plus an identifier they
  // become a stray statement in the output.
  for (const std::string_view literal : {"0D", "20260312D", "0T", "120000T", "0DT"}) {
    const auto tokens = Tokenize(std::string("x := ") + std::string(literal) + ";");
    CHECK_TRUE(std::string("a date literal is one token: ") + std::string(literal),
               tokens.size() == 5 && tokens[2].kind == TokenKind::DateTime);
  }
  const auto timeWithFraction = Tokenize("x := 235959.999T;");
  CHECK_TRUE("and so is a fractional time", timeWithFraction[2].kind == TokenKind::DateTime);

  // The BigInteger suffix carries no meaning for us but must be CONSUMED; unconsumed it landed in
  // openerp's output as a bare `l` on its own line.
  const auto big = Tokenize("x := 10000L;");
  CHECK_TRUE("a BigInteger suffix is consumed", big.size() == 5);
  CHECK_TEXT("and the value survives it", big[2].text, "10000");

  const auto range = Tokenize("x in [1 .. 5]");
  const bool hasRange = std::ranges::any_of(range, [](const auto &t) { return t.text == ".."; });
  CHECK_TRUE("the range operator is one token", hasRange);
  CHECK_TRUE("and a digit before it is still a whole number",
             std::ranges::any_of(range, [](const auto &t) { return t.text == "1"; }));
}

/// THE PREPROCESSOR IS NOT SKIPPED LINE BY LINE, and that is the sharpest of the lessons: BC's
/// obsolete guard brackets a procedure SIGNATURE in `#if`/`#else` with an equivalent head in both
/// branches and one shared body after `#endif`. Lexing both heads mis-parses.
///
/// The symbol model is the predecessor's, measured against the whole BaseApp: EVERY conditional
/// symbol is undefined, because this is not a CLEANxx build. So `not CLEAN27` keeps the legacy
/// branch and `CLEAN27` drops the clean-only one.
void ThePreprocessorKeepsOneBranch() {
  using agiru::al::Tokenize;

  const auto legacy = Tokenize("#if not CLEAN27\nold\n#else\nclean\n#endif\n");
  CHECK_TRUE("`not CLEANxx` keeps the legacy branch",
             legacy.size() == 2 && legacy[0].text == "old");

  const auto clean = Tokenize("#if CLEAN27\nclean\n#else\nold\n#endif\n");
  CHECK_TRUE("`CLEANxx` alone keeps the else branch", clean.size() == 2 && clean[0].text == "old");

  const auto plain = Tokenize("#if CLEAN27\nclean\n#endif\nafter\n");
  CHECK_TRUE("an unguarded `#if CLEANxx` body drops entirely",
             plain.size() == 2 && plain[0].text == "after");

  const auto pragma = Tokenize("#pragma warning disable AA0074\nkept\n");
  CHECK_TRUE("a pragma is not a branch and takes nothing with it",
             pragma.size() == 2 && pragma[0].text == "kept");
}

} // namespace

int main() {
  return gate::Run("AlParser", [] {
    const TableObject table = ParseTable(kSource);
    TheObjectHeaderIsRead(table);
    EveryFieldCarriesItsDeclaration(table);
    AnOptionCarriesItsMembers(table);
    ATriggerIsCapturedWhole(table);
    TheKeysAreRead(table);
    TheLabelsAreRead(table);
    TheLexerReadsWhatBaseAppActuallyContains();
    ThePreprocessorKeepsOneBranch();
    TheRealFileParses();
  });
}
