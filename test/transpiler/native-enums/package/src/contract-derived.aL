namespace System.Fixture;

interface "Native Contract" extends "Native Base"
{
    procedure Echo(var Value: Enum "Native Sparse"): Enum "Native Sparse";
}
