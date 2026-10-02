namespace Microsoft.Fixture;

codeunit 50199 "Text Guid Oracle"
{
    procedure Positive()
    var
        Identity: Guid;
        Prefix: Text[2];
        Suffix: Text;
        UnitCode: Code[2];
        Result: Text;
    begin
        Prefix := ' x';
        Suffix := 'y ';
        UnitCode := 'ab';
        Result := Prefix + Identity;
        Result := Identity + Prefix;
        Result := Suffix + Identity;
        Result := Identity + Suffix;
        Result := Prefix + Identity + Suffix + UnitCode;
        Result := 'Configured new external BC company:' + UserSecurityId() + ', ' + UnitCode;
    end;

#if NEGATIVE
    procedure RevealResultTypes()
    var
        Identity: Guid;
        Bounded: Text[2];
        Unbounded: Text;
        Result: Boolean;
    begin
        Result := (Bounded + Identity) = true;
        Result := (Identity + Bounded) = true;
        Result := (Unbounded + Identity) = true;
        Result := (Identity + Unbounded) = true;
    end;
#endif

#if SECRET_ASSIGNMENT
    procedure RefuseImplicitSecretAssignment()
    var
        Plain: Text;
        Secret: SecretText;
    begin
        Secret := Plain;
    end;
#endif

#if SECRET_LITERAL
    procedure RefuseImplicitSecretLiteral()
    var
        Secret: SecretText;
    begin
        Secret := 'secret';
    end;
#endif

#if ADDITIONAL_TYPES
    procedure RevealAdditionalTypes()
    var
        LeftCode: Code[2];
        RightCode: Code[2];
        Identity: Guid;
        Result: Boolean;
    begin
        Result := (LeftCode + RightCode) = true;
        Result := ('a' + 'b') = true;
        Result := ('a' + Identity) = true;
        Result := (Identity + 'b') = true;
    end;
#endif
}
