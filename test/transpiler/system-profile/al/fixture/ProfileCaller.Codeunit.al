namespace Microsoft.Fixture;

codeunit 50199 ProfileCaller
{
    procedure Write(var Row: Record ProfileRow): BigInteger
    begin
        Row.ID := 1;
        Row.Insert();
        Row.Modify();
        exit(Row.SystemRowVersion);
    end;
}
