namespace Microsoft.Fixture;

codeunit 50194 "Native Privacy"
{
    procedure Exercise(): Integer
    var
        Notice: Record "Privacy Notice" temporary;
        Approval: Record "Privacy Notice Approval" temporary;
        Reflected: RecordRef;
        User: Guid;
        OtherUser: Guid;
    begin
        if Notice.FieldNo("User SID Filter") <> 4 then Error('Original filter number');
        if Notice.FieldNo(Enabled) <> 5 then Error('Original enabled number');
        if Notice.FieldNo(Disabled) <> 6 then Error('Original disabled number');
        Notice.ID := 'notice-a';
        Notice."Integration Service Name" := ' Mixed.Case Service ';
        Notice.Link := 'https://example.com/Privacy?Key=Mixed';
        Reflected.GetTable(Notice);
        if Reflected.Number() <> 2000000237 then Error('Original notice identity');
        if Reflected.Field(3).Value() <> 'https://example.com/Privacy?Key=Mixed' then Error('Text Link reflection');
        if not Notice.Insert() then Error('Temporary notice insert');
        Notice.ID := 'notice-b';
        Notice."Integration Service Name" := 'Other Service';
        if not Notice.Insert() then Error('Second temporary notice insert');
        if Notice.Count() <> 2 then Error('Temporary notice population');
        if not Notice.SetCurrentKey("Integration Service Name") then Error('Original secondary key');
        Notice.SetRange("Integration Service Name", ' Mixed.Case Service ');
        if not Notice.FindFirst() then Error('Text service lookup');
        if Notice.ID <> 'NOTICE-A' then Error('Original notice row');
        User := CreateGuid();
        OtherUser := CreateGuid();
        Approval.ID := Notice.ID;
        Approval."User SID" := User;
        Approval."Approver User SID" := OtherUser;
        Approval.Approved := true;
        Reflected.GetTable(Approval);
        if Reflected.Number() <> 2000000238 then Error('Original approval identity');
        if not Approval.Insert() then Error('First temporary approval insert');
        Approval."User SID" := OtherUser;
        Approval.Approved := false;
        if not Approval.Insert() then Error('Approval key retains user');
        if Approval.Count() <> 2 then Error('Temporary approval population');
        Approval.SetRange("User SID", User);
        if not Approval.FindFirst() then Error('User-specific approval lookup');
        if not Approval.Approved then Error('Original approval value');
        if Approval."Approver User SID" <> OtherUser then Error('Distinct approver identity');
        exit(18);
    end;
}
