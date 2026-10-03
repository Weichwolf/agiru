namespace Microsoft.Fixture;

codeunit 50250 "Test Context Consumer"
{
    procedure Provider(Context: DataSourceContext): Integer
    begin
        exit(Context.CodeunitId);
    end;

    procedure TestApp(Context: DataSourceContext): Guid
    begin
        exit(Context.AppId());
    end;

    procedure Identity(Context: TestHandlerContext): Text
    begin
        exit(StrSubstNo('%1|%2|%3|%4|%5', Context.CodeunitId,
            Context.CodeunitName(), Context.ProcedureName, Context.TestCaseName(), Context.Success));
    end;

    procedure CopySkip(Context: TestHandlerContext)
    var
        Copy: TestHandlerContext;
    begin
        Copy := Context;
        Copy.Skip('requested by AL copy');
    end;
}
