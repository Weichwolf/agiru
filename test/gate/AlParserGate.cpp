#include "Ast.h"
#include "Check.h"
#include "Expr.h"
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

/// Parses one procedure body and hands back its statements. The grammar cases below are about
/// STATEMENTS and EXPRESSIONS, and wrapping each in its own table would bury what they claim.
std::vector<agiru::al::Stmt> OnlyBody(std::string_view body) {
  const std::string source =
      "codeunit 50000 X\n{\n    procedure P()\n    " + std::string(body) + "\n}\n";
  const agiru::al::CodeunitObject unit = agiru::al::ParseCodeunit(source);
  return unit.procedures.at(0).body;
}

/// The single expression of a one-statement body, which for `X := ...` is the assignment.
agiru::al::Expr OnlyExpression(std::string_view statement) {
  return OnlyBody("begin " + std::string(statement) + " end;").at(0).expression;
}

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

/// A `var` BLOCK ENDS WHERE THE NEXT MEMBER BEGINS, and the next member usually begins with its
/// attributes. Reading one to find that out threw it away: every test codeunit in BCApps lost
/// exactly one `[Test]`, and the loss was invisible because the count came from the same parser
/// that had dropped it. 1 115 methods across the tree, 67 of them in the milestone's own
/// population.
void AVarBlockDoesNotSwallowTheNextMembersAttribute() {
  const agiru::al::CodeunitObject unit = agiru::al::ParseCodeunit(R"(
codeunit 50000 "Something UT"
{
    Subtype = Test;

    var
        Assert: Codeunit Assert;
        IsInitialized: Boolean;

    [Test]
    procedure TheFirstOne()
    begin
        Assert.IsTrue(true, '');
    end;

    [Test]
    [HandlerFunctions('ConfirmHandler')]
    procedure TheSecondOne()
    begin
        Assert.IsTrue(true, '');
    end;

    procedure NotATest()
    begin
        Assert.IsTrue(true, '');
    end;
})");

  CHECK_TRUE("all three procedures parse", unit.procedures.size() == 3);
  CHECK_TRUE("the one right after the var block keeps its [Test]",
             agiru::al::HasAttribute(unit.procedures[0], "Test"));
  CHECK_TRUE("and so does the next", agiru::al::HasAttribute(unit.procedures[1], "Test"));
  CHECK_TRUE("whose second attribute survives too",
             agiru::al::HasAttribute(unit.procedures[1], "HandlerFunctions"));
  // THE NEGATIVE CONTROL. A rule that simply kept every attribute would also mark the third.
  CHECK_TRUE("a procedure with no attribute is not marked",
             !agiru::al::HasAttribute(unit.procedures[2], "Test"));
  // And the var block itself is still read: the labels/variables it declares are not members.
  CHECK_TRUE("the var block did not become a procedure", unit.procedures.size() == 3);
}

/// THE AL PRECEDENCE IS PASCAL'S, NOT C'S, and this parser had C's until it was checked against
/// `c-al-operators.md`, which states the hierarchy outright:
///
///     3. *  /  DIV  MOD  AND  XOR      4. +  -  OR      5. >  <  >=  <=  =  <>  IN
///
/// AND binds like multiplication, OR like addition, and the comparisons bind LOOSEST.
void TheOperatorHierarchyIsTheOneCalDocuments() {
  // The documentation's own example: this evaluates to 14, not 20.
  const agiru::al::Expr sum = OnlyExpression("X := 2 + 3 * 4;");
  CHECK_TEXT("multiplication binds tighter than addition", sum.children[1].text, "+");
  CHECK_TEXT("so the product is the right operand", sum.children[1].children[1].text, "*");

  // `A = B and C` is `A = (B and C)` in AL and `(A = B) and C` in C. The tree says which.
  const agiru::al::Expr mixed = OnlyExpression("X := A = B and C;");
  CHECK_TEXT("the comparison is the outermost operator", mixed.children[1].text, "=");
  CHECK_TEXT("and the conjunction is beneath it", mixed.children[1].children[1].text, "and");

  // Which is the OPPOSITE of what a C reading gives, and that is the negative control: a parser
  // that still had C's table would put `and` on top and `=` beneath.
  CHECK_TRUE("a C reading would have inverted them", mixed.children[1].text != "and");

  const agiru::al::Expr disjunction = OnlyExpression("X := A + B or C;");
  CHECK_TEXT("OR sits with addition, so it is left-associative with it",
             disjunction.children[1].text,
             "or");
}

/// `xor` is an AL operator and 10 codeunits use it.
void ExclusiveDisjunctionIsAnOperator() {
  const agiru::al::Expr e = OnlyExpression("X := (A < 0) xor (B < 0);");
  CHECK_TEXT("xor parses as a binary operator", e.children[1].text, "xor");
  CHECK_TRUE("with both sides", e.children[1].children.size() == 2);
}

/// BC 25 gave AL a conditional operator, and the BaseApp uses it.
void TheConditionalOperatorParses() {
  const agiru::al::Expr e = OnlyExpression("X := Setup.Get() ? A : B;");
  CHECK_TEXT("it is one expression with three parts", e.children[1].text, "?:");
  CHECK_TRUE("condition, then, else", e.children[1].children.size() == 3);
  CHECK_TRUE("and it binds loosest, so the condition is the whole call",
             e.children[1].children[0].kind == agiru::al::ExprKind::Call);
}

/// `asserterror <statement>` -- one word in front of an ordinary statement, and a test suite is
/// mostly made of it. Its absence cost 3 codeunits, and it was the last parse failure in BCApps.
void AssertErrorIsAStatement() {
  const std::vector<agiru::al::Stmt> body =
      OnlyBody("begin asserterror Rec.Get(1); Rec.Insert(); end;");
  CHECK_TRUE("two statements", body.size() == 2);
  CHECK_TRUE("the first is an asserterror", body[0].kind == agiru::al::StmtKind::AssertError);
  CHECK_TRUE("carrying the statement it expects to raise", body[0].body.size() == 1);
  // THE NEGATIVE CONTROL. Without the keyword the same line is an ordinary call, and the parser
  // must not turn every call into an expectation.
  const std::vector<agiru::al::Stmt> plain = OnlyBody("begin Rec.Get(1); end;");
  CHECK_TRUE("a bare call is not an asserterror",
             plain[0].kind != agiru::al::StmtKind::AssertError);
}

/// A BINARY NODE WITH THREE CHILDREN. AL's conditional operator is the only one, and without a case
/// of its own the emitter's chain walk found no chain, left its cursor on the node it started from,
/// and called itself -- recursion with no bottom. It surfaced as nine objects "nesting too deeply"
/// and, with the depth guard lifted to measure it, as a segmentation fault.
void TheConditionalOperatorHasThreeOperandsAndOneTranslation() {
  const agiru::al::Expr e = OnlyExpression("X := A ? B : C;");
  CHECK_TRUE("three children and not two", e.children[1].children.size() == 3);
  CHECK_TEXT("and the node says which operator it is", e.children[1].text, "?:");
  // It is right-associative, so a nested one lands in the ELSE branch rather than the then branch.
  const agiru::al::Expr nested = OnlyExpression("X := A ? B : C ? D : E;");
  CHECK_TEXT("a chained conditional nests to the right", nested.children[1].children[2].text, "?:");
  CHECK_TRUE("and the then branch stays simple",
             nested.children[1].children[1].kind == agiru::al::ExprKind::Name);
}

/// A SEMICOLON ENDS THE `if`, AND THE `else` AFTER ONE BELONGS TO THE ENCLOSING `case`.
///
/// `Incoming Document.GetRelatedDocType` is the case that found it: the last branch before the
/// case's `else` ends in `if ... then exit(...);`, the `if` took the else for itself, and the
/// branch parser then ran off the end of the tokens looking for the next label. One table of 1 809
/// stopped parsing, and it was a CODEUNIT defect that had never surfaced because no codeunit in the
/// read roots writes the shape.
void AnElseAfterASemicolonBelongsToTheCase() {
  const std::vector<agiru::al::Stmt> body =
      OnlyBody("begin case true of A: if B then exit(1); else exit(2); end; end;");
  CHECK_TRUE("one statement, the case", body.size() == 1);
  CHECK_TRUE("which is a case", body[0].kind == agiru::al::StmtKind::Case);
  CHECK_TRUE("and it carries the else", !body[0].otherwise.empty());
  const agiru::al::Stmt &branch = body[0].body.front();
  CHECK_TRUE("the branch holds the if", branch.body.front().kind == agiru::al::StmtKind::If);
  CHECK_TRUE("and the if did NOT take the else", branch.body.front().otherwise.empty());
  // THE NEGATIVE CONTROL. Without the semicolon the else is the if's own, and the rule must not
  // take every else away from every if.
  const std::vector<agiru::al::Stmt> plain = OnlyBody("begin if B then exit(1) else exit(2); end;");
  CHECK_TRUE("an if with no semicolon keeps its else", !plain[0].otherwise.empty());
}

/// AN EXTENSION HAS THE SAME BODY AS THE OBJECT IT EXTENDS, which is why one routine reads both.
///
/// BC merges an extension at BUILD time -- the added columns land in the same SQL table -- so
/// merging it in the transpiler is faithful rather than a shortcut, and a C++ class is closed
/// anyway (board:0033).
void AnExtensionCarriesWhatItAdds() {
  const agiru::al::TableExtensionObject table = agiru::al::ParseTableExtension(
      R"(tableextension 50000 "More Item" extends Item
{
    fields
    {
        field(50000; "Shelf Depth"; Decimal) { Caption = 'Shelf Depth'; }
        modify("No.") { Editable = false; }
    }
    keys { key(Extra; "Shelf Depth") { } }
    procedure Deeper(): Decimal
    begin
        exit("Shelf Depth");
    end;
})");
  CHECK_TEXT("it names the table it extends", table.extends, "Item");
  CHECK_TRUE("one field is added", table.fields.size() == 1);
  CHECK_TEXT("with its own number", std::to_string(table.fields[0].number), "50000");
  // A MODIFIED FIELD IS NOT AN ADDED ONE. It carries no type, only what it changes, and merging it
  // as a field would give the table a second `No.` with number 0.
  CHECK_TRUE("and the modified one is kept apart", table.modified.size() == 1);
  CHECK_TEXT("naming the field it changes", table.modified[0].name, "No.");
  CHECK_TRUE("a key is added", table.keys.size() == 1);
  CHECK_TRUE("and so is code", table.procedures.size() == 1);

  const agiru::al::EnumExtensionObject values = agiru::al::ParseEnumExtension(
      R"(enumextension 50000 "More Types" extends "Item Type"
{
    value(50000; Rental) { Caption = 'Rental'; }
    value(50001; Leased) { Caption = 'Leased'; }
})");
  CHECK_TEXT("an enumextension names its enumeration", values.extends, "Item Type");
  CHECK_TRUE("and carries its values with their own ordinals", values.values.size() == 2);
  CHECK_TEXT("the first of them", values.values[0].name, "Rental");
  CHECK_TEXT("with the ordinal AL wrote", std::to_string(values.values[0].ordinal), "50000");

  const agiru::al::PageExtensionObject page = agiru::al::ParsePageExtension(
      R"(pageextension 50000 "More Item Card" extends "Item Card"
{
    layout
    {
        addafter(Description)
        {
            field("Shelf Depth"; Rec."Shelf Depth") { ApplicationArea = All; }
        }
    }
    actions { addlast(processing) { action(Measure) { trigger OnAction() begin end; } } }
    procedure Measured(): Boolean
    begin
        exit(true);
    end;
})");
  CHECK_TEXT("a pageextension names its page", page.extends, "Item Card");
  CHECK_TRUE("the layout operation reads as a control", page.layout.size() == 1);
  CHECK_TEXT("and says which one it is", page.layout[0].kind, "addafter");
  CHECK_TRUE("carrying the control it adds", page.layout[0].children.size() == 1);
  CHECK_TRUE("the actions likewise", page.actions.size() == 1);
  CHECK_TRUE("and the code comes with it", page.procedures.size() == 1);
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
    AVarBlockDoesNotSwallowTheNextMembersAttribute();
    TheOperatorHierarchyIsTheOneCalDocuments();
    ExclusiveDisjunctionIsAnOperator();
    TheConditionalOperatorParses();
    AssertErrorIsAStatement();
    AnElseAfterASemicolonBelongsToTheCase();
    AnExtensionCarriesWhatItAdds();
    TheConditionalOperatorHasThreeOperandsAndOneTranslation();
  });
}
