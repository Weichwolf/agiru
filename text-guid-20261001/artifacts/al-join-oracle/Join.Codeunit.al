namespace Microsoft.Fixture;

codeunit 50199 "Text Guid Join"
{
    procedure Exercise(): Integer
    var
        Identity: Guid;
        EmptyIdentity: Guid;
        Prefix: Text[2];
        Suffix: Text;
        Unicode: Text[3];
        UnitCode: Code[2];
        Secret: SecretText;
    begin
        Identity := 'aaaaaaaa-0000-1111-2222-bbbbbbbbbbbb';
        Prefix := ' x';
        Suffix := 'y ';
        Unicode := 'ä💡';
        UnitCode := 'ab';
        if Prefix + Identity <> ' x{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}' then Error('Bounded prefix');
        if Identity + Prefix <> '{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB} x' then Error('Bounded suffix');
        if Suffix + Identity <> 'y {AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}' then Error('Unbounded prefix');
        if Identity + Suffix <> '{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}y ' then Error('Unbounded suffix');
        if Prefix + Identity + Suffix + UnitCode <> ' x{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}y AB' then Error('Text Guid Code chain');
        if 'a' + Identity + 'b' <> 'a{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}b' then Error('Literal Guid chain');
        if Suffix + EmptyIdentity <> 'y {00000000-0000-0000-0000-000000000000}' then Error('Null Guid');
        if StrLen(Prefix + Identity) <> 40 then Error('No input bound on result');
        if StrLen(Unicode + Identity) <> 41 then Error('UTF-16 length');
        if Unicode + Identity <> 'ä💡{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}' then Error('Unicode');
        if Which(Prefix + Identity) <> 1 then Error('Bounded Text Guid overload');
        if Which(Identity + Prefix) <> 1 then Error('Guid bounded Text overload');
        if Which(Suffix + Identity) <> 1 then Error('Unbounded Text Guid overload');
        if Which(Identity + Suffix) <> 1 then Error('Guid unbounded Text overload');
        if Which(Identity) <> 2 then Error('Guid remains Guid');
        if Which(Secret) <> 3 then Error('Secret remains Secret');
        exit(16);
    end;

    procedure Limited(): Text[3]
    var
        Identity: Guid;
        Prefix: Text[2];
        Destination: Text[3];
    begin
        Prefix := ' x';
        Destination := Prefix + Identity;
        exit(Destination);
    end;

    local procedure Which(Value: Text): Integer begin exit(1); end;
    local procedure Which(Value: Guid): Integer begin exit(2); end;
    local procedure Which(Value: SecretText): Integer begin exit(3); end;
}
