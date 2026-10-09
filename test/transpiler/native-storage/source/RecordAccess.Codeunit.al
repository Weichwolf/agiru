codeunit 50179 "Stored Record Access"
{
    procedure Called(): Boolean
    var
        EntityText: Record "Entity Text";
    begin
        exit(EntityText.ReadPermission());
    end;

    procedure Property(): Boolean
    var
        EntityText: Record "Entity Text";
    begin
        exit(EntityText.ReadPermission);
    end;
}
