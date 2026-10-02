namespace Microsoft.Fixture;

codeunit 50196 "Native Link"
{
    procedure Exercise(): Integer
    var
        Related: Record "All Profile" temporary;
        Link: Record "Record Link" temporary;
        Ordinary: Record "Collision Source" temporary;
        Own: RecordId;
        Target: RecordId;
        Reflected: RecordRef;
    begin
        Related."Profile ID" := 'related';
        Target := Related.RecordId();
        Link."Link ID" := 7;
        Link."Record ID" := Target;
        Link.URL1 := PadStr('u', 2048, 'u');
        Link."User ID" := PadStr('Creator.Mixed Case', 132, 'x');
        Link."To User ID" := PadStr('Recipient.Mixed Case', 132, 'y');
        Link.Description := PadStr('d', 250, 'd');
        Link.Company := 'Company.Mixed';
        Link.Type := Link.Type::Note;
        Link.Notify := true;
        if not Link.Insert() then Error('Native temporary insert');
        Link.Reset();
        if not Link.FindFirst() then Error('Native temporary lookup');
        if StrLen(Link.URL1) <> 2048 then Error('Source URL1 length');
        if Link."User ID" <> PadStr('Creator.Mixed Case', 132, 'x') then Error('Creator Text preserves all characters');
        if Link."To User ID" <> PadStr('Recipient.Mixed Case', 132, 'y') then Error('Recipient Text preserves all characters');
        if StrLen(Link.Description) <> 250 then Error('Source description length');
        if Link.Company <> 'Company.Mixed' then Error('Company Text case');
        if Link.Type <> Link.Type::Note then Error('Source note option');
        if not Link.Notify then Error('Source notification field');
        if Link."Record ID" <> Target then Error('Quoted field preserves related record');
        Own := Link.RecordId;
        if Own.TableNo() <> 2000000068 then Error('Implicit native method is not related field');
        Own := Link.RecordID;
        if Own.TableNo() <> 2000000068 then Error('Uppercase native implicit method');
        Own := Link.recordid;
        if Own.TableNo() <> 2000000068 then Error('Lowercase native implicit method');
        Own := Link.RECORDID;
        if Own.TableNo() <> 2000000068 then Error('Capital native implicit method');
        Own := Link.RecordId();
        if Own.TableNo() <> 2000000068 then Error('Explicit native method');
        Own := Link.RecordID();
        if Own.TableNo() <> 2000000068 then Error('Uppercase native explicit method');
        Own := Link.recordid();
        if Own.TableNo() <> 2000000068 then Error('Lowercase native explicit method');
        Own := Link.RECORDID();
        if Own.TableNo() <> 2000000068 then Error('Capital native explicit method');
        if Own = Target then Error('Own and related identities stay distinct');
        if Link."Record ID" <> Target then Error('Calling the method never mutates the related field');
        Reflected.GetTable(Link);
        if Reflected.KeyCount() <> 3 then Error('All declared keys');
        if not Reflected.FieldExist(14) then Error('Original recipient field is reflected');
        Own := Reflected.RecordId();
        if Own.TableNo() <> 2000000068 then Error('Reflected current identity');
        Ordinary."Entry No." := 9;
        Ordinary."Record ID" := Target;
        if not Ordinary.Insert() then Error('Ordinary temporary insert');
        Own := Ordinary.RecordId;
        if Own.TableNo() <> 50197 then Error('Ordinary implicit method');
        Own := Ordinary.RecordID;
        if Own.TableNo() <> 50197 then Error('Ordinary uppercase implicit method');
        Own := Ordinary.RecordId();
        if Own.TableNo() <> 50197 then Error('Ordinary explicit method');
        if Ordinary."Record ID" <> Target then Error('Ordinary quoted field is not its own identity');
        exit(28);
    end;
}
