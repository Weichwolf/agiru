namespace Microsoft.Fixture;

codeunit 50195 "Native Profile"
{
    procedure Exercise(): Integer
    var
        Profile: Record "All Profile" temporary;
        Reflected: RecordRef;
    begin
        if Profile.FieldNo("Use Comments") <> 7 then Error('Original comments number');
        if Profile.FieldNo("Use Notes") <> 8 then Error('Original notes number');
        if Profile.FieldNo("Use Record Notes") <> 9 then Error('Original record notes number');
        if Profile.FieldNo("Record Notebook") <> 10 then Error('Original record notebook number');
        if Profile.FieldNo("Use Page Notes") <> 11 then Error('Original page notes number');
        if Profile.FieldNo("Page Notebook") <> 12 then Error('Original page notebook number');
        if Profile.FieldNo("Disable Personalization") <> 13 then Error('Original personalization number');
        if Profile.FieldNo("App Name") <> 14 then Error('Original app name number');
        if Profile.FieldNo(Enabled) <> 15 then Error('Original enabled number');
        if Profile.FieldNo(Caption) <> 16 then Error('Original caption number');
        if Profile.FieldNo(Promoted) <> 17 then Error('Original promoted number');
        Profile.Scope := Profile.Scope::System;
        Profile."Profile ID" := 'profile-a';
        Profile.Description := PadStr('d', 2048, 'd');
        Profile."App Name" := PadStr('a', 250, 'a');
        Profile.Caption := PadStr('c', 100, 'c');
        Profile."Record Notebook" := 'Record Notebook';
        Profile."Page Notebook" := 'Page Notebook';
        Profile."Disable Personalization" := true;
        Profile.Enabled := true;
        Profile.Promoted := true;
        if not Profile.Insert() then Error('Temporary profile insert');
        Profile.Reset();
        if not Profile.FindFirst() then Error('Temporary profile lookup');
        if StrLen(Profile.Description) <> 2048 then Error('Full source description');
        if StrLen(Profile."App Name") <> 250 then Error('Independent app name length');
        if StrLen(Profile.Caption) <> 100 then Error('Independent caption length');
        if Profile."Record Notebook" <> 'Record Notebook' then Error('Record notebook retained');
        if Profile."Page Notebook" <> 'Page Notebook' then Error('Page notebook retained');
        if not Profile."Disable Personalization" then Error('Personalization retained');
        if not Profile.Enabled then Error('Enabled retained');
        if not Profile.Promoted then Error('Promoted retained');
        Reflected.GetTable(Profile);
        if Reflected.KeyCount() <> 1 then Error('Only the original primary key');
        if Reflected.Number() <> 2000000178 then Error('Original table identity');
        if Reflected.FieldCount() <> 17 then Error('All declared fields');
        exit(24);
    end;
}
