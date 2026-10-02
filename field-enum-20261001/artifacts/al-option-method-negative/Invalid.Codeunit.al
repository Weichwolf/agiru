namespace Microsoft.Fixture;
using System.Reflection;
codeunit 50223 Invalid {
  procedure Read(var Row: Record AllObjWithCaption): Integer
  begin exit(Row."Object Type".AsInteger()); end;
}
