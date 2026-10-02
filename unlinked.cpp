// Generated from the slice's undefined symbols. Do not edit.
// One definition per AL procedure the slice calls and no linked source defines.

#include "runtime/Error.h"

#include <string>

namespace {

[[noreturn]] void Unlinked(const char *procedure) {
  throw ::agiru::Error(std::string("the AL procedure ") + procedure +
                       " is declared and its source is not in the slice (board:0612)");
}

}

extern "C" void agiru_unlinked_0() asm("_ZN5agiru10Foundation4Task27UserTaskManagement_Codeunit13CompleteTasksERNS1_14UserTask_TableE");
extern "C" void agiru_unlinked_0() { Unlinked("agiru::Foundation::Task::UserTaskManagement_Codeunit::CompleteTasks(agiru::Foundation::Task::UserTask_Table&)"); }
extern "C" void agiru_unlinked_1() asm("_ZN5agiru10Foundation4Task27UserTaskManagement_Codeunit26GetMyPendingUserTasksCountEv");
extern "C" void agiru_unlinked_1() { Unlinked("agiru::Foundation::Task::UserTaskManagement_Codeunit::GetMyPendingUserTasksCount()"); }
extern "C" void agiru_unlinked_2() asm("_ZN5agiru10Foundation4Task27UserTaskManagement_Codeunit27SetFiltersToShowMyUserTasksERNS1_14UserTask_TableEi");
extern "C" void agiru_unlinked_2() { Unlinked("agiru::Foundation::Task::UserTaskManagement_Codeunit::SetFiltersToShowMyUserTasks(agiru::Foundation::Task::UserTask_Table&, int)"); }
extern "C" void agiru_unlinked_3() asm("_ZN5agiru10Foundation4Task27UserTaskManagement_Codeunit34GetMyPendingUserTasksCountDueTodayEv");
extern "C" void agiru_unlinked_3() { Unlinked("agiru::Foundation::Task::UserTaskManagement_Codeunit::GetMyPendingUserTasksCountDueToday()"); }
extern "C" void agiru_unlinked_4() asm("_ZN5agiru10Foundation4Task27UserTaskManagement_Codeunit37GetMyPendingUserTasksCountDueThisWeekEv");
extern "C" void agiru_unlinked_4() { Unlinked("agiru::Foundation::Task::UserTaskManagement_Codeunit::GetMyPendingUserTasksCountDueThisWeek()"); }
extern "C" void agiru_unlinked_5() asm("_ZN5agiru10Foundation4Task27UserTaskManagement_Codeunit7FindRecERNS1_14UserTask_TableES4_NS_4TextILm0EEE");
extern "C" void agiru_unlinked_5() { Unlinked("agiru::Foundation::Task::UserTaskManagement_Codeunit::FindRec(agiru::Foundation::Task::UserTask_Table&, agiru::Foundation::Task::UserTask_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_6() asm("_ZN5agiru10Foundation4Task27UserTaskManagement_Codeunit7NextRecERNS1_14UserTask_TableES4_i");
extern "C" void agiru_unlinked_6() { Unlinked("agiru::Foundation::Task::UserTaskManagement_Codeunit::NextRec(agiru::Foundation::Task::UserTask_Table&, agiru::Foundation::Task::UserTask_Table&, int)"); }
extern "C" void agiru_unlinked_7() asm("_ZN5agiru10Foundation7Company14Companies_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_7() { Unlinked("agiru::Foundation::Company::Companies_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_8() asm("_ZN5agiru10Foundation7Company14Companies_Page11OnNewRecordEb");
extern "C" void agiru_unlinked_8() { Unlinked("agiru::Foundation::Company::Companies_Page::OnNewRecord(bool)"); }
extern "C" void agiru_unlinked_9() asm("_ZN5agiru10Foundation7Company14Companies_Page14OnInsertRecordEb");
extern "C" void agiru_unlinked_9() { Unlinked("agiru::Foundation::Company::Companies_Page::OnInsertRecord(bool)"); }
extern "C" void agiru_unlinked_10() asm("_ZN5agiru10Foundation7Company14Companies_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_10() { Unlinked("agiru::Foundation::Company::Companies_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_11() asm("_ZN5agiru10Foundation7Company14Companies_Page19OnActionCopyCompanyEv");
extern "C" void agiru_unlinked_11() { Unlinked("agiru::Foundation::Company::Companies_Page::OnActionCopyCompany()"); }
extern "C" void agiru_unlinked_12() asm("_ZN5agiru10Foundation7Company14Companies_Page22OnDrillDownSetupStatusEv");
extern "C" void agiru_unlinked_12() { Unlinked("agiru::Foundation::Company::Companies_Page::OnDrillDownSetupStatus()"); }
extern "C" void agiru_unlinked_13() asm("_ZN5agiru10Foundation7Company14Companies_Page24OnActionCreateNewCompanyEv");
extern "C" void agiru_unlinked_13() { Unlinked("agiru::Foundation::Company::Companies_Page::OnActionCreateNewCompany()"); }
extern "C" void agiru_unlinked_14() asm("_ZN5agiru10Foundation7Company14Companies_Page24OnEditableCompanyNameVarEv");
extern "C" void agiru_unlinked_14() { Unlinked("agiru::Foundation::Company::Companies_Page::OnEditableCompanyNameVar()"); }
extern "C" void agiru_unlinked_15() asm("_ZN5agiru10Foundation7Company14Companies_Page24OnValidateCompanyNameVarEv");
extern "C" void agiru_unlinked_15() { Unlinked("agiru::Foundation::Company::Companies_Page::OnValidateCompanyNameVar()"); }
extern "C" void agiru_unlinked_16() asm("_ZN5agiru10Foundation7Company14Companies_Page26OnVisibleEvaluationCompanyEv");
extern "C" void agiru_unlinked_16() { Unlinked("agiru::Foundation::Company::Companies_Page::OnVisibleEvaluationCompany()"); }
extern "C" void agiru_unlinked_17() asm("_ZN5agiru10Foundation7Company14Companies_Page36OnValidateEnableAssistedCompanySetupEv");
extern "C" void agiru_unlinked_17() { Unlinked("agiru::Foundation::Company::Companies_Page::OnValidateEnableAssistedCompanySetup()"); }
extern "C" void agiru_unlinked_18() asm("_ZN5agiru10Foundation7Company14Companies_Page6OnInitEv");
extern "C" void agiru_unlinked_18() { Unlinked("agiru::Foundation::Company::Companies_Page::OnInit()"); }
extern "C" void agiru_unlinked_19() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_19() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_20() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page11OnClosePageEv");
extern "C" void agiru_unlinked_20() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnClosePage()"); }
extern "C" void agiru_unlinked_21() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page14OnValidateIBANEv");
extern "C" void agiru_unlinked_21() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateIBAN()"); }
extern "C" void agiru_unlinked_22() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page16OnActionAction27Ev");
extern "C" void agiru_unlinked_22() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionAction27()"); }
extern "C" void agiru_unlinked_23() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page16OnActionNoSeriesEv");
extern "C" void agiru_unlinked_23() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionNoSeries()"); }
extern "C" void agiru_unlinked_24() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page17OnActionJobsSetupEv");
extern "C" void agiru_unlinked_24() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionJobsSetup()"); }
extern "C" void agiru_unlinked_25() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page17OnActionLanguagesEv");
extern "C" void agiru_unlinked_25() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionLanguages()"); }
extern "C" void agiru_unlinked_26() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page17OnActionPostCodesEv");
extern "C" void agiru_unlinked_26() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionPostCodes()"); }
extern "C" void agiru_unlinked_27() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page17OnValidatePictureEv");
extern "C" void agiru_unlinked_27() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidatePicture()"); }
extern "C" void agiru_unlinked_28() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page18OnVisibleReportingEv");
extern "C" void agiru_unlinked_28() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnVisibleReporting()"); }
extern "C" void agiru_unlinked_29() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page19OnActionReasonCodesEv");
extern "C" void agiru_unlinked_29() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionReasonCodes()"); }
extern "C" void agiru_unlinked_30() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page19OnActionSourceCodesEv");
extern "C" void agiru_unlinked_30() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionSourceCodes()"); }
extern "C" void agiru_unlinked_31() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page19OnValidateSWIFTCodeEv");
extern "C" void agiru_unlinked_31() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateSWIFTCode()"); }
extern "C" void agiru_unlinked_32() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_32() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_33() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page20OnVisibleCountyGroupEv");
extern "C" void agiru_unlinked_33() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnVisibleCountyGroup()"); }
extern "C" void agiru_unlinked_34() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page21OnActionReportLayoutsEv");
extern "C" void agiru_unlinked_34() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionReportLayouts()"); }
extern "C" void agiru_unlinked_35() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page21OnVisibleShipToCountyEv");
extern "C" void agiru_unlinked_35() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnVisibleShipToCounty()"); }
extern "C" void agiru_unlinked_36() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page22OnActionInventorySetupEv");
extern "C" void agiru_unlinked_36() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionInventorySetup()"); }
extern "C" void agiru_unlinked_37() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page22OnActionOnlineMapSetupEv");
extern "C" void agiru_unlinked_37() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionOnlineMapSetup()"); }
extern "C" void agiru_unlinked_38() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page22OnActionPermissionSetsEv");
extern "C" void agiru_unlinked_38() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionPermissionSets()"); }
extern "C" void agiru_unlinked_39() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page22OnAssistEditExperienceEv");
extern "C" void agiru_unlinked_39() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnAssistEditExperience()"); }
extern "C" void agiru_unlinked_40() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page22OnValidateBankBranchNoEv");
extern "C" void agiru_unlinked_40() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateBankBranchNo()"); }
extern "C" void agiru_unlinked_41() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page22OnValidateCompanyBadgeEv");
extern "C" void agiru_unlinked_41() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateCompanyBadge()"); }
extern "C" void agiru_unlinked_42() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page23OnValidateBankAccountNoEv");
extern "C" void agiru_unlinked_42() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateBankAccountNo()"); }
extern "C" void agiru_unlinked_43() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page24OnActionCountriesRegionsEv");
extern "C" void agiru_unlinked_43() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionCountriesRegions()"); }
extern "C" void agiru_unlinked_44() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page24OnActionFixedAssetsSetupEv");
extern "C" void agiru_unlinked_44() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionFixedAssetsSetup()"); }
extern "C" void agiru_unlinked_45() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page26OnActionGeneralLedgerSetupEv");
extern "C" void agiru_unlinked_45() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionGeneralLedgerSetup()"); }
extern "C" void agiru_unlinked_46() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page26OnValidateDefaultThemePartEv");
extern "C" void agiru_unlinked_46() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateDefaultThemePart()"); }
extern "C" void agiru_unlinked_47() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page27OnActionHumanResourcesSetupEv");
extern "C" void agiru_unlinked_47() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionHumanResourcesSetup()"); }
extern "C" void agiru_unlinked_48() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page27OnValidateCountryRegionCodeEv");
extern "C" void agiru_unlinked_48() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateCountryRegionCode()"); }
extern "C" void agiru_unlinked_49() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page28OnAssistEditDefaultThemePartEv");
extern "C" void agiru_unlinked_49() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnAssistEditDefaultThemePart()"); }
extern "C" void agiru_unlinked_50() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page28OnDrillDownVATRegistrationNoEv");
extern "C" void agiru_unlinked_50() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnDrillDownVATRegistrationNo()"); }
extern "C" void agiru_unlinked_51() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page29OnActionResponsibilityCentersEv");
extern "C" void agiru_unlinked_51() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionResponsibilityCenters()"); }
extern "C" void agiru_unlinked_52() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page29OnActionSalesReceivablesSetupEv");
extern "C" void agiru_unlinked_52() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionSalesReceivablesSetup()"); }
extern "C" void agiru_unlinked_53() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page29OnDrillDownCustomizedCalendarEv");
extern "C" void agiru_unlinked_53() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnDrillDownCustomizedCalendar()"); }
extern "C" void agiru_unlinked_54() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page29OnEditableSystemIndicatorTextEv");
extern "C" void agiru_unlinked_54() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnEditableSystemIndicatorText()"); }
extern "C" void agiru_unlinked_55() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page29OnValidateSystemIndicatorTextEv");
extern "C" void agiru_unlinked_55() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateSystemIndicatorText()"); }
extern "C" void agiru_unlinked_56() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page30OnActionPurchasesPayablesSetupEv");
extern "C" void agiru_unlinked_56() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnActionPurchasesPayablesSetup()"); }
extern "C" void agiru_unlinked_57() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page30OnValidateSystemIndicatorStyleEv");
extern "C" void agiru_unlinked_57() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateSystemIndicatorStyle()"); }
extern "C" void agiru_unlinked_58() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page33OnValidateBankAccountPostingGroupEv");
extern "C" void agiru_unlinked_58() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateBankAccountPostingGroup()"); }
extern "C" void agiru_unlinked_59() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page33OnValidateDefaultHeaderFooterPartEv");
extern "C" void agiru_unlinked_59() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateDefaultHeaderFooterPart()"); }
extern "C" void agiru_unlinked_60() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page33OnValidateShipToCountryRegionCodeEv");
extern "C" void agiru_unlinked_60() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnValidateShipToCountryRegionCode()"); }
extern "C" void agiru_unlinked_61() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page35OnAssistEditDefaultHeaderFooterPartEv");
extern "C" void agiru_unlinked_61() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnAssistEditDefaultHeaderFooterPart()"); }
extern "C" void agiru_unlinked_62() asm("_ZN5agiru10Foundation7Company23CompanyInformation_Page6OnInitEv");
extern "C" void agiru_unlinked_62() { Unlinked("agiru::Foundation::Company::CompanyInformation_Page::OnInit()"); }
extern "C" void agiru_unlinked_63() asm("_ZN5agiru10Foundation8NoSeries28NoSeriesCopilotImpl_Codeunit11FeatureNameEv");
extern "C" void agiru_unlinked_63() { Unlinked("agiru::Foundation::NoSeries::NoSeriesCopilotImpl_Codeunit::FeatureName()"); }
extern "C" void agiru_unlinked_64() asm("_ZN5agiru10Foundation8NoSeries28NoSeriesCopilotImpl_Codeunit16IsCopilotVisibleEv");
extern "C" void agiru_unlinked_64() { Unlinked("agiru::Foundation::NoSeries::NoSeriesCopilotImpl_Codeunit::IsCopilotVisible()"); }
extern "C" void agiru_unlinked_65() asm("_ZN5agiru10Foundation8NoSeries28NoSeriesCopilotImpl_Codeunit16SendNotificationENS_4TextILm0EEE");
extern "C" void agiru_unlinked_65() { Unlinked("agiru::Foundation::NoSeries::NoSeriesCopilotImpl_Codeunit::SendNotification(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_66() asm("_ZN5agiru10Foundation8NoSeries28NoSeriesCopilotImpl_Codeunit22ApplyGeneratedNoSeriesERNS1_30NoSeriesGenerationDetail_TableE");
extern "C" void agiru_unlinked_66() { Unlinked("agiru::Foundation::NoSeries::NoSeriesCopilotImpl_Codeunit::ApplyGeneratedNoSeries(agiru::Foundation::NoSeries::NoSeriesGenerationDetail_Table&)"); }
extern "C" void agiru_unlinked_67() asm("_ZN5agiru10Foundation8NoSeries28NoSeriesCopilotImpl_Codeunit22GetNoSeriesSuggestionsEv");
extern "C" void agiru_unlinked_67() { Unlinked("agiru::Foundation::NoSeries::NoSeriesCopilotImpl_Codeunit::GetNoSeriesSuggestions()"); }
extern "C" void agiru_unlinked_68() asm("_ZN5agiru10Foundation8NoSeries28NoSeriesCopilotImpl_Codeunit28GetChatCompletionResponseErrEv");
extern "C" void agiru_unlinked_68() { Unlinked("agiru::Foundation::NoSeries::NoSeriesCopilotImpl_Codeunit::GetChatCompletionResponseErr()"); }
extern "C" void agiru_unlinked_69() asm("_ZN5agiru10Foundation8NoSeries28NoSeriesCopilotImpl_Codeunit8GenerateERNS1_24NoSeriesGeneration_TableERNS1_30NoSeriesGenerationDetail_TableENS_4TextILm0EEE");
extern "C" void agiru_unlinked_69() { Unlinked("agiru::Foundation::NoSeries::NoSeriesCopilotImpl_Codeunit::Generate(agiru::Foundation::NoSeries::NoSeriesGeneration_Table&, agiru::Foundation::NoSeries::NoSeriesGenerationDetail_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_70() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_70() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_71() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_71() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_72() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page16OnLookupReportIDERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_72() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnLookupReportID(agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_73() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page18OnValidateReportIDEv");
extern "C" void agiru_unlinked_73() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnValidateReportID()"); }
extern "C" void agiru_unlinked_74() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_74() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_75() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page21OnValidateCompanyNameEv");
extern "C" void agiru_unlinked_75() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnValidateCompanyName()"); }
extern "C" void agiru_unlinked_76() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page23OnActionSetForOneLayoutEv");
extern "C" void agiru_unlinked_76() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnActionSetForOneLayout()"); }
extern "C" void agiru_unlinked_77() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page23OnActionSetForOneReportEv");
extern "C" void agiru_unlinked_77() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnActionSetForOneReport()"); }
extern "C" void agiru_unlinked_78() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page24OnActionSetForAllReportsEv");
extern "C" void agiru_unlinked_78() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnActionSetForAllReports()"); }
extern "C" void agiru_unlinked_79() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page25OnActionWidenToAllLayoutsEv");
extern "C" void agiru_unlinked_79() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnActionWidenToAllLayouts()"); }
extern "C" void agiru_unlinked_80() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page26OnEnabledWidenToAllLayoutsEv");
extern "C" void agiru_unlinked_80() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnEnabledWidenToAllLayouts()"); }
extern "C" void agiru_unlinked_81() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page28OnAssistEditThemePartDisplayEv");
extern "C" void agiru_unlinked_81() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnAssistEditThemePartDisplay()"); }
extern "C" void agiru_unlinked_82() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page29OnAssistEditHeaderPartDisplayEv");
extern "C" void agiru_unlinked_82() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnAssistEditHeaderPartDisplay()"); }
extern "C" void agiru_unlinked_83() asm("_ZN5agiru10Foundation9Reporting26TenantReportLayoutCfg_Page29OnAssistEditLayoutNameDisplayEv");
extern "C" void agiru_unlinked_83() { Unlinked("agiru::Foundation::Reporting::TenantReportLayoutCfg_Page::OnAssistEditLayoutNameDisplay()"); }
extern "C" void agiru_unlinked_84() asm("_ZN5agiru10Foundation9Reporting32CompositeReportPartsMgt_Codeunit16SeedDefaultPartsEv");
extern "C" void agiru_unlinked_84() { Unlinked("agiru::Foundation::Reporting::CompositeReportPartsMgt_Codeunit::SeedDefaultParts()"); }
extern "C" void agiru_unlinked_85() asm("_ZN5agiru10Foundation9Reporting32CompositeReportPartsMgt_Codeunit19GetShippedPartAppIdEv");
extern "C" void agiru_unlinked_85() { Unlinked("agiru::Foundation::Reporting::CompositeReportPartsMgt_Codeunit::GetShippedPartAppId()"); }
extern "C" void agiru_unlinked_86() asm("_ZN5agiru10Foundation9Reporting32CompositeReportPartsMgt_Codeunit8SeedPartENS_4TextILm250EEENS3_ILm0EEENS_4EnumIvEES5_");
extern "C" void agiru_unlinked_86() { Unlinked("agiru::Foundation::Reporting::CompositeReportPartsMgt_Codeunit::SeedPart(agiru::Text<250ul>, agiru::Text<0ul>, agiru::Enum<void>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_87() asm("_ZN5agiru11Integration5Graph34GraphCollectionMgtContact_Codeunit11SplitStreetENS_4TextILm0EEERS4_S5_");
extern "C" void agiru_unlinked_87() { Unlinked("agiru::Integration::Graph::GraphCollectionMgtContact_Codeunit::SplitStreet(agiru::Text<0ul>, agiru::Text<0ul>&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_88() asm("_ZN5agiru11Integration5Graph34GraphCollectionMgtContact_Codeunit17ConcatenateStreetENS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_88() { Unlinked("agiru::Integration::Graph::GraphCollectionMgtContact_Codeunit::ConcatenateStreet(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_89() asm("_ZN5agiru11Integration9Dataverse27SyntheticRelations_Codeunit23DeleteSyntheticRelationENS_6absent26SynthRelationMappingBufferE");
extern "C" void agiru_unlinked_89() { Unlinked("agiru::Integration::Dataverse::SyntheticRelations_Codeunit::DeleteSyntheticRelation(agiru::absent::SynthRelationMappingBuffer)"); }
extern "C" void agiru_unlinked_90() asm("_ZN5agiru11Integration9Dataverse27SyntheticRelations_Codeunit23GetFeatureTelemetryNameEv");
extern "C" void agiru_unlinked_90() { Unlinked("agiru::Integration::Dataverse::SyntheticRelations_Codeunit::GetFeatureTelemetryName()"); }
extern "C" void agiru_unlinked_91() asm("_ZN5agiru11Integration9Dataverse27SyntheticRelations_Codeunit28LoadExistingBCTableRelationsERNS_6absent26SynthRelationMappingBufferE");
extern "C" void agiru_unlinked_91() { Unlinked("agiru::Integration::Dataverse::SyntheticRelations_Codeunit::LoadExistingBCTableRelations(agiru::absent::SynthRelationMappingBuffer&)"); }
extern "C" void agiru_unlinked_92() asm("_ZN5agiru11Integration9Dataverse28NewSyntheticRelationWiz_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_92() { Unlinked("agiru::Integration::Dataverse::NewSyntheticRelationWiz_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_93() asm("_ZN5agiru11Integration9Dataverse28NewSyntheticRelationWiz_Page27SetExistingBCTableRelationsERNS_6absent26SynthRelationMappingBufferE");
extern "C" void agiru_unlinked_93() { Unlinked("agiru::Integration::Dataverse::NewSyntheticRelationWiz_Page::SetExistingBCTableRelations(agiru::absent::SynthRelationMappingBuffer&)"); }
extern "C" void agiru_unlinked_94() asm("_ZN5agiru11Integration9Dataverse28NewSyntheticRelationWiz_Page6OnInitEv");
extern "C" void agiru_unlinked_94() { Unlinked("agiru::Integration::Dataverse::NewSyntheticRelationWiz_Page::OnInit()"); }
extern "C" void agiru_unlinked_95() asm("_ZN5agiru11RoleCenters30RolecenterSelectorMgt_Codeunit28BuildJsonFromPageActionTableEi");
extern "C" void agiru_unlinked_95() { Unlinked("agiru::RoleCenters::RolecenterSelectorMgt_Codeunit::BuildJsonFromPageActionTable(int)"); }
extern "C" void agiru_unlinked_96() asm("_ZN5agiru11RoleCenters30RolecenterSelectorMgt_Codeunit38BuildPageDataJsonForRolecenterSelectorEv");
extern "C" void agiru_unlinked_96() { Unlinked("agiru::RoleCenters::RolecenterSelectorMgt_Codeunit::BuildPageDataJsonForRolecenterSelector()"); }
extern "C" void agiru_unlinked_97() asm("_ZN5agiru12Intercompany12DataExchange35CrossIntercompanyConnector_Codeunit18TestICPartnerSetupERNS0_7Partner15ICPartner_TableE");
extern "C" void agiru_unlinked_97() { Unlinked("agiru::Intercompany::DataExchange::CrossIntercompanyConnector_Codeunit::TestICPartnerSetup(agiru::Intercompany::Partner::ICPartner_Table&)"); }
extern "C" void agiru_unlinked_98() asm("_ZN5agiru12Intercompany12DataExchange35CrossIntercompanyConnector_Codeunit20FinishICPartnerSetupERNS0_7Partner15ICPartner_TableE");
extern "C" void agiru_unlinked_98() { Unlinked("agiru::Intercompany::DataExchange::CrossIntercompanyConnector_Codeunit::FinishICPartnerSetup(agiru::Intercompany::Partner::ICPartner_Table&)"); }
extern "C" void agiru_unlinked_99() asm("_ZN5agiru12Intercompany12DataExchange35CrossIntercompanyConnector_Codeunit27RequestICPartnerBankAccountENS0_7Partner15ICPartner_TableE");
extern "C" void agiru_unlinked_99() { Unlinked("agiru::Intercompany::DataExchange::CrossIntercompanyConnector_Codeunit::RequestICPartnerBankAccount(agiru::Intercompany::Partner::ICPartner_Table)"); }
extern "C" void agiru_unlinked_100() asm("_ZN5agiru12Intercompany12DataExchange35CrossIntercompanyConnector_Codeunit30NotifyICPartnerFromBoundActionENS0_7Partner15ICPartner_TableENS_4GuidENS_4TextILm0EEE");
extern "C" void agiru_unlinked_100() { Unlinked("agiru::Intercompany::DataExchange::CrossIntercompanyConnector_Codeunit::NotifyICPartnerFromBoundAction(agiru::Intercompany::Partner::ICPartner_Table, agiru::Guid, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_101() asm("_ZN5agiru12Intercompany12DataExchange35CrossIntercompanyConnector_Codeunit32RemoveCurlyBracketsAndUpperCasesENS_4GuidE");
extern "C" void agiru_unlinked_101() { Unlinked("agiru::Intercompany::DataExchange::CrossIntercompanyConnector_Codeunit::RemoveCurlyBracketsAndUpperCases(agiru::Guid)"); }
extern "C" void agiru_unlinked_102() asm("_ZN5agiru12Intercompany12DataExchange35CrossIntercompanyConnector_Codeunit34RequestICPartnerCompanyInformationENS0_7Partner15ICPartner_TableE");
extern "C" void agiru_unlinked_102() { Unlinked("agiru::Intercompany::DataExchange::CrossIntercompanyConnector_Codeunit::RequestICPartnerCompanyInformation(agiru::Intercompany::Partner::ICPartner_Table)"); }
extern "C" void agiru_unlinked_103() asm("_ZN5agiru12Intercompany12DataExchange35CrossIntercompanyConnector_Codeunit34RequestICPartnerGeneralLedgerSetupENS0_7Partner15ICPartner_TableE");
extern "C" void agiru_unlinked_103() { Unlinked("agiru::Intercompany::DataExchange::CrossIntercompanyConnector_Codeunit::RequestICPartnerGeneralLedgerSetup(agiru::Intercompany::Partner::ICPartner_Table)"); }
extern "C" void agiru_unlinked_104() asm("_ZN5agiru12Intercompany12DataExchange35CrossIntercompanyConnector_Codeunit37RequestICPartnerRecordsFromEntityNameENS0_7Partner15ICPartner_TableENS_4TextILm0EEE");
extern "C" void agiru_unlinked_104() { Unlinked("agiru::Intercompany::DataExchange::CrossIntercompanyConnector_Codeunit::RequestICPartnerRecordsFromEntityName(agiru::Intercompany::Partner::ICPartner_Table, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_105() asm("_ZN5agiru12Intercompany12DataExchange35CrossIntercompanyConnector_Codeunit38RequestICPartnerICOutgoingNotificationENS0_7Partner15ICPartner_TableENS_4GuidERNS_10JsonObjectE");
extern "C" void agiru_unlinked_105() { Unlinked("agiru::Intercompany::DataExchange::CrossIntercompanyConnector_Codeunit::RequestICPartnerICOutgoingNotification(agiru::Intercompany::Partner::ICPartner_Table, agiru::Guid, agiru::JsonObject&)"); }
extern "C" void agiru_unlinked_106() asm("_ZN5agiru12Intercompany12DataExchange35CrossIntercompanyConnector_Codeunit38SubmitRecordsToICPartnerFromEntityNameENS0_7Partner15ICPartner_TableENS_4TextILm0EEES6_S6_S6_");
extern "C" void agiru_unlinked_106() { Unlinked("agiru::Intercompany::DataExchange::CrossIntercompanyConnector_Codeunit::SubmitRecordsToICPartnerFromEntityName(agiru::Intercompany::Partner::ICPartner_Table, agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_107() asm("_ZN5agiru13Manufacturing6Wizard36ProductionDefinitionManager_Codeunit12RunForSourceENS_7VariantENS_4EnumINS1_23ProdDefinitionMode_EnumEEE");
extern "C" void agiru_unlinked_107() { Unlinked("agiru::Manufacturing::Wizard::ProductionDefinitionManager_Codeunit::RunForSource(agiru::Variant, agiru::Enum<agiru::Manufacturing::Wizard::ProdDefinitionMode_Enum>)"); }
extern "C" void agiru_unlinked_108() asm("_ZN5agiru13Manufacturing6Wizard36ProductionDefinitionManager_Codeunit12RunForSourceENS_7VariantENS_4EnumINS1_23ProdDefinitionMode_EnumEEENS4_INS0_8Document26ProductionOrderStatus_EnumEEE");
extern "C" void agiru_unlinked_108() { Unlinked("agiru::Manufacturing::Wizard::ProductionDefinitionManager_Codeunit::RunForSource(agiru::Variant, agiru::Enum<agiru::Manufacturing::Wizard::ProdDefinitionMode_Enum>, agiru::Enum<agiru::Manufacturing::Document::ProductionOrderStatus_Enum>)"); }
extern "C" void agiru_unlinked_109() asm("_ZN5agiru15ExportData_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_109() { Unlinked("agiru::ExportData_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_110() asm("_ZN5agiru15ExportData_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_110() { Unlinked("agiru::ExportData_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_111() asm("_ZN5agiru15ExportData_Page16OnQueryClosePageENS_6ActionE");
extern "C" void agiru_unlinked_111() { Unlinked("agiru::ExportData_Page::OnQueryClosePage(agiru::Action)"); }
extern "C" void agiru_unlinked_112() asm("_ZN5agiru15ExportData_Page6OnInitEv");
extern "C" void agiru_unlinked_112() { Unlinked("agiru::ExportData_Page::OnInit()"); }
extern "C" void agiru_unlinked_113() asm("_ZN5agiru16AccountantPortal33InviteExternalAccountant_Codeunit13CreateNewUserENS_4GuidE");
extern "C" void agiru_unlinked_113() { Unlinked("agiru::AccountantPortal::InviteExternalAccountant_Codeunit::CreateNewUser(agiru::Guid)"); }
extern "C" void agiru_unlinked_114() asm("_ZN5agiru16AccountantPortal33InviteExternalAccountant_Codeunit19UpdateAssistedSetupEv");
extern "C" void agiru_unlinked_114() { Unlinked("agiru::AccountantPortal::InviteExternalAccountant_Codeunit::UpdateAssistedSetup()"); }
extern "C" void agiru_unlinked_115() asm("_ZN5agiru16AccountantPortal33InviteExternalAccountant_Codeunit20TryGetGuestGraphUserENS_4GuidERNS_6dotnet8UserInfoE");
extern "C" void agiru_unlinked_115() { Unlinked("agiru::AccountantPortal::InviteExternalAccountant_Codeunit::TryGetGuestGraphUser(agiru::Guid, agiru::dotnet::UserInfo&)"); }
extern "C" void agiru_unlinked_116() asm("_ZN5agiru16AccountantPortal33InviteExternalAccountant_Codeunit24InvokeInvitationsRequestENS_4TextILm0EEES3_S3_RNS_4GuidERS3_S6_");
extern "C" void agiru_unlinked_116() { Unlinked("agiru::AccountantPortal::InviteExternalAccountant_Codeunit::InvokeInvitationsRequest(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>, agiru::Guid&, agiru::Text<0ul>&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_117() asm("_ZN5agiru16AccountantPortal33InviteExternalAccountant_Codeunit24IsLicenseAlreadyAssignedENS_6dotnet8UserInfoE");
extern "C" void agiru_unlinked_117() { Unlinked("agiru::AccountantPortal::InviteExternalAccountant_Codeunit::IsLicenseAlreadyAssigned(agiru::dotnet::UserInfo)"); }
extern "C" void agiru_unlinked_118() asm("_ZN5agiru16AccountantPortal33InviteExternalAccountant_Codeunit25InvokeIsUserAdministratorEv");
extern "C" void agiru_unlinked_118() { Unlinked("agiru::AccountantPortal::InviteExternalAccountant_Codeunit::InvokeIsUserAdministrator()"); }
extern "C" void agiru_unlinked_119() asm("_ZN5agiru16AccountantPortal33InviteExternalAccountant_Codeunit29SendTelemetryForWizardFailureENS_4TextILm0EEES3_");
extern "C" void agiru_unlinked_119() { Unlinked("agiru::AccountantPortal::InviteExternalAccountant_Codeunit::SendTelemetryForWizardFailure(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_120() asm("_ZN5agiru16AccountantPortal33InviteExternalAccountant_Codeunit30InvokeUserAssignLicenseRequestERNS_6dotnet8UserInfoENS_4TextILm0EEERS6_");
extern "C" void agiru_unlinked_120() { Unlinked("agiru::AccountantPortal::InviteExternalAccountant_Codeunit::InvokeUserAssignLicenseRequest(agiru::dotnet::UserInfo&, agiru::Text<0ul>, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_121() asm("_ZN5agiru16AccountantPortal33InviteExternalAccountant_Codeunit30InvokeUserProfileUpdateRequestERNS_6dotnet8UserInfoENS_4TextILm0EEERS6_");
extern "C" void agiru_unlinked_121() { Unlinked("agiru::AccountantPortal::InviteExternalAccountant_Codeunit::InvokeUserProfileUpdateRequest(agiru::dotnet::UserInfo&, agiru::Text<0ul>, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_122() asm("_ZN5agiru16AccountantPortal33InviteExternalAccountant_Codeunit42InvokeIsExternalAccountantLicenseAvailableERNS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_122() { Unlinked("agiru::AccountantPortal::InviteExternalAccountant_Codeunit::InvokeIsExternalAccountantLicenseAvailable(agiru::Text<0ul>&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_123() asm("_ZN5agiru19O365Activities_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_123() { Unlinked("agiru::O365Activities_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_124() asm("_ZN5agiru19O365Activities_Page15OnVisibleCameraEv");
extern "C" void agiru_unlinked_124() { Unlinked("agiru::O365Activities_Page::OnVisibleCamera()"); }
extern "C" void agiru_unlinked_125() asm("_ZN5agiru19O365Activities_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_125() { Unlinked("agiru::O365Activities_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_126() asm("_ZN5agiru19O365Activities_Page16OnVisibleWelcomeEv");
extern "C" void agiru_unlinked_126() { Unlinked("agiru::O365Activities_Page::OnVisibleWelcome()"); }
extern "C" void agiru_unlinked_127() asm("_ZN5agiru19O365Activities_Page17OnActionSetUpCuesEv");
extern "C" void agiru_unlinked_127() { Unlinked("agiru::O365Activities_Page::OnActionSetUpCues()"); }
extern "C" void agiru_unlinked_128() asm("_ZN5agiru19O365Activities_Page19OnActionRefreshDataEv");
extern "C" void agiru_unlinked_128() { Unlinked("agiru::O365Activities_Page::OnActionRefreshData()"); }
extern "C" void agiru_unlinked_129() asm("_ZN5agiru19O365Activities_Page19OnVisibleGetStartedEv");
extern "C" void agiru_unlinked_129() { Unlinked("agiru::O365Activities_Page::OnVisibleGetStarted()"); }
extern "C" void agiru_unlinked_130() asm("_ZN5agiru19O365Activities_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_130() { Unlinked("agiru::O365Activities_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_131() asm("_ZN5agiru19O365Activities_Page21OnVisibleIntercompanyEv");
extern "C" void agiru_unlinked_131() { Unlinked("agiru::O365Activities_Page::OnVisibleIntercompany()"); }
extern "C" void agiru_unlinked_132() asm("_ZN5agiru19O365Activities_Page24OnVisibleDataIntegrationEv");
extern "C" void agiru_unlinked_132() { Unlinked("agiru::O365Activities_Page::OnVisibleDataIntegration()"); }
extern "C" void agiru_unlinked_133() asm("_ZN5agiru19O365Activities_Page25OnDrillDownSalesThisMonthEv");
extern "C" void agiru_unlinked_133() { Unlinked("agiru::O365Activities_Page::OnDrillDownSalesThisMonth()"); }
extern "C" void agiru_unlinked_134() asm("_ZN5agiru19O365Activities_Page26OnActionGettingStartedTileEv");
extern "C" void agiru_unlinked_134() { Unlinked("agiru::O365Activities_Page::OnActionGettingStartedTile()"); }
extern "C" void agiru_unlinked_135() asm("_ZN5agiru19O365Activities_Page28OnActionReplayGettingStartedEv");
extern "C" void agiru_unlinked_135() { Unlinked("agiru::O365Activities_Page::OnActionReplayGettingStarted()"); }
extern "C" void agiru_unlinked_136() asm("_ZN5agiru19O365Activities_Page28OnActionShowStartInMyCompanyEv");
extern "C" void agiru_unlinked_136() { Unlinked("agiru::O365Activities_Page::OnActionShowStartInMyCompany()"); }
extern "C" void agiru_unlinked_137() asm("_ZN5agiru19O365Activities_Page28OnVisibleAwaitingVerficationEv");
extern "C" void agiru_unlinked_137() { Unlinked("agiru::O365Activities_Page::OnVisibleAwaitingVerfication()"); }
extern "C" void agiru_unlinked_138() asm("_ZN5agiru19O365Activities_Page28OnVisibleICInboxTransactionsEv");
extern "C" void agiru_unlinked_138() { Unlinked("agiru::O365Activities_Page::OnVisibleICInboxTransactions()"); }
extern "C" void agiru_unlinked_139() asm("_ZN5agiru19O365Activities_Page29OnDrillDownNonAppliedPaymentsEv");
extern "C" void agiru_unlinked_139() { Unlinked("agiru::O365Activities_Page::OnDrillDownNonAppliedPayments()"); }
extern "C" void agiru_unlinked_140() asm("_ZN5agiru19O365Activities_Page29OnVisibleCDSIntegrationErrorsEv");
extern "C" void agiru_unlinked_140() { Unlinked("agiru::O365Activities_Page::OnVisibleCDSIntegrationErrors()"); }
extern "C" void agiru_unlinked_141() asm("_ZN5agiru19O365Activities_Page29OnVisibleICOutboxTransactionsEv");
extern "C" void agiru_unlinked_141() { Unlinked("agiru::O365Activities_Page::OnVisibleICOutboxTransactions()"); }
extern "C" void agiru_unlinked_142() asm("_ZN5agiru19O365Activities_Page30OnDrillDownAwaitingVerficationEv");
extern "C" void agiru_unlinked_142() { Unlinked("agiru::O365Activities_Page::OnDrillDownAwaitingVerfication()"); }
extern "C" void agiru_unlinked_143() asm("_ZN5agiru19O365Activities_Page31OnVisibleCoupledDataSynchErrorsEv");
extern "C" void agiru_unlinked_143() { Unlinked("agiru::O365Activities_Page::OnVisibleCoupledDataSynchErrors()"); }
extern "C" void agiru_unlinked_144() asm("_ZN5agiru19O365Activities_Page31OnVisibleSalesCrMPendingDocExchEv");
extern "C" void agiru_unlinked_144() { Unlinked("agiru::O365Activities_Page::OnVisibleSalesCrMPendingDocExch()"); }
extern "C" void agiru_unlinked_145() asm("_ZN5agiru19O365Activities_Page31OnVisibleSalesInvPendingDocExchEv");
extern "C" void agiru_unlinked_145() { Unlinked("agiru::O365Activities_Page::OnVisibleSalesInvPendingDocExch()"); }
extern "C" void agiru_unlinked_146() asm("_ZN5agiru19O365Activities_Page32OnDrillDownSOrdReservedFromStockEv");
extern "C" void agiru_unlinked_146() { Unlinked("agiru::O365Activities_Page::OnDrillDownSOrdReservedFromStock()"); }
extern "C" void agiru_unlinked_147() asm("_ZN5agiru19O365Activities_Page32OnVisibleDocumentExchangeServiceEv");
extern "C" void agiru_unlinked_147() { Unlinked("agiru::O365Activities_Page::OnVisibleDocumentExchangeService()"); }
extern "C" void agiru_unlinked_148() asm("_ZN5agiru19O365Activities_Page36OnDrillDownOverduePurchInvoiceAmountEv");
extern "C" void agiru_unlinked_148() { Unlinked("agiru::O365Activities_Page::OnDrillDownOverduePurchInvoiceAmount()"); }
extern "C" void agiru_unlinked_149() asm("_ZN5agiru19O365Activities_Page36OnDrillDownOverdueSalesInvoiceAmountEv");
extern "C" void agiru_unlinked_149() { Unlinked("agiru::O365Activities_Page::OnDrillDownOverdueSalesInvoiceAmount()"); }
extern "C" void agiru_unlinked_150() asm("_ZN5agiru19O365Activities_Page40OnActionCreateIncomingDocumentFromCameraEv");
extern "C" void agiru_unlinked_150() { Unlinked("agiru::O365Activities_Page::OnActionCreateIncomingDocumentFromCamera()"); }
extern "C" void agiru_unlinked_151() asm("_ZN5agiru19O365Activities_Page6OnInitEv");
extern "C" void agiru_unlinked_151() { Unlinked("agiru::O365Activities_Page::OnInit()"); }
extern "C" void agiru_unlinked_152() asm("_ZN5agiru22BackupStorage_Codeunit10MaxBackupsEv");
extern "C" void agiru_unlinked_152() { Unlinked("agiru::BackupStorage_Codeunit::MaxBackups()"); }
extern "C" void agiru_unlinked_153() asm("_ZN5agiru22BackupStorage_Codeunit10TaintTableEiib");
extern "C" void agiru_unlinked_153() { Unlinked("agiru::BackupStorage_Codeunit::TaintTable(int, int, bool)"); }
extern "C" void agiru_unlinked_154() asm("_ZN5agiru22BackupStorage_Codeunit11SetWorkDateEv");
extern "C" void agiru_unlinked_154() { Unlinked("agiru::BackupStorage_Codeunit::SetWorkDate()"); }
extern "C" void agiru_unlinked_155() asm("_ZN5agiru22BackupStorage_Codeunit14DeleteBackupNoEi");
extern "C" void agiru_unlinked_155() { Unlinked("agiru::BackupStorage_Codeunit::DeleteBackupNo(int)"); }
extern "C" void agiru_unlinked_156() asm("_ZN5agiru22BackupStorage_Codeunit14GetTableBackupEiiRNS_9RecordRefE");
extern "C" void agiru_unlinked_156() { Unlinked("agiru::BackupStorage_Codeunit::GetTableBackup(int, int, agiru::RecordRef&)"); }
extern "C" void agiru_unlinked_157() asm("_ZN5agiru22BackupStorage_Codeunit16GetTaintedTablesERNS_18TaintedTable_TableE");
extern "C" void agiru_unlinked_157() { Unlinked("agiru::BackupStorage_Codeunit::GetTaintedTables(agiru::TaintedTable_Table&)"); }
extern "C" void agiru_unlinked_158() asm("_ZN5agiru22BackupStorage_Codeunit18ClearImplicitTaintEii");
extern "C" void agiru_unlinked_158() { Unlinked("agiru::BackupStorage_Codeunit::ClearImplicitTaint(int, int)"); }
extern "C" void agiru_unlinked_159() asm("_ZN5agiru22BackupStorage_Codeunit19BackupTableRowCountEii");
extern "C" void agiru_unlinked_159() { Unlinked("agiru::BackupStorage_Codeunit::BackupTableRowCount(int, int)"); }
extern "C" void agiru_unlinked_160() asm("_ZN5agiru22BackupStorage_Codeunit20BackupTableIsTaintedEii");
extern "C" void agiru_unlinked_160() { Unlinked("agiru::BackupStorage_Codeunit::BackupTableIsTainted(int, int)"); }
extern "C" void agiru_unlinked_161() asm("_ZN5agiru22BackupStorage_Codeunit20RestoreTaintedTablesEib");
extern "C" void agiru_unlinked_161() { Unlinked("agiru::BackupStorage_Codeunit::RestoreTaintedTables(int, bool)"); }
extern "C" void agiru_unlinked_162() asm("_ZN5agiru22BackupStorage_Codeunit21BackupTableInBackupNoEiNS_4TextILm1024EEEi");
extern "C" void agiru_unlinked_162() { Unlinked("agiru::BackupStorage_Codeunit::BackupTableInBackupNo(int, agiru::Text<1024ul>, int)"); }
extern "C" void agiru_unlinked_163() asm("_ZN5agiru22BackupStorage_Codeunit24RestoreTableFromBackupNoEiNS_4TextILm1024EEEi");
extern "C" void agiru_unlinked_163() { Unlinked("agiru::BackupStorage_Codeunit::RestoreTableFromBackupNo(int, agiru::Text<1024ul>, int)"); }
extern "C" void agiru_unlinked_164() asm("_ZN5agiru22BackupStorage_Codeunit28GetDatabaseTableTriggerSetupEi");
extern "C" void agiru_unlinked_164() { Unlinked("agiru::BackupStorage_Codeunit::GetDatabaseTableTriggerSetup(int)"); }
extern "C" void agiru_unlinked_165() asm("_ZN5agiru22BackupStorage_Codeunit7IsEmptyEi");
extern "C" void agiru_unlinked_165() { Unlinked("agiru::BackupStorage_Codeunit::IsEmpty(int)"); }
extern "C" void agiru_unlinked_166() asm("_ZN5agiru22XmlAttributeCollection3GetESt17basic_string_viewIcSt11char_traitsIcEES4_RNS_12XmlAttributeE");
extern "C" void agiru_unlinked_166() { Unlinked("agiru::XmlAttributeCollection::Get(std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, agiru::XmlAttribute&)"); }
extern "C" void agiru_unlinked_167() asm("_ZN5agiru23LibraryXMLRead_Codeunit10InitializeENS_4TextILm0EEE");
extern "C" void agiru_unlinked_167() { Unlinked("agiru::LibraryXMLRead_Codeunit::Initialize(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_168() asm("_ZN5agiru23LibraryXMLRead_Codeunit15VerifyNodeValueENS_4TextILm0EEENS_7VariantE");
extern "C" void agiru_unlinked_168() { Unlinked("agiru::LibraryXMLRead_Codeunit::VerifyNodeValue(agiru::Text<0ul>, agiru::Variant)"); }
extern "C" void agiru_unlinked_169() asm("_ZN5agiru23LibraryXMLRead_Codeunit17VerifyNodeAbsenceENS_4TextILm0EEE");
extern "C" void agiru_unlinked_169() { Unlinked("agiru::LibraryXMLRead_Codeunit::VerifyNodeAbsence(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_170() asm("_ZN5agiru23LibraryXMLRead_Codeunit19GetNodeValueAtIndexENS_4TextILm0EEEi");
extern "C" void agiru_unlinked_170() { Unlinked("agiru::LibraryXMLRead_Codeunit::GetNodeValueAtIndex(agiru::Text<0ul>, int)"); }
extern "C" void agiru_unlinked_171() asm("_ZN5agiru23LibraryXMLRead_Codeunit24VerifyNodeValueInSubtreeENS_4TextILm0EEES2_NS_7VariantE");
extern "C" void agiru_unlinked_171() { Unlinked("agiru::LibraryXMLRead_Codeunit::VerifyNodeValueInSubtree(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Variant)"); }
extern "C" void agiru_unlinked_172() asm("_ZN5agiru23LibraryXMLRead_Codeunit26GetAttributeValueInSubtreeENS_4TextILm0EEES2_S2_");
extern "C" void agiru_unlinked_172() { Unlinked("agiru::LibraryXMLRead_Codeunit::GetAttributeValueInSubtree(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_173() asm("_ZN5agiru23LibraryXMLRead_Codeunit29VerifyAttributeValueInSubtreeENS_4TextILm0EEES2_S2_S2_");
extern "C" void agiru_unlinked_173() { Unlinked("agiru::LibraryXMLRead_Codeunit::VerifyAttributeValueInSubtree(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_174() asm("_ZN5agiru23LibraryXMLRead_Codeunit29VerifyElementAbsenceInSubtreeENS_4TextILm0EEES2_");
extern "C" void agiru_unlinked_174() { Unlinked("agiru::LibraryXMLRead_Codeunit::VerifyElementAbsenceInSubtree(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_175() asm("_ZN5agiru23LibraryXMLRead_Codeunit31VerifyAttributeAbsenceInSubtreeENS_4TextILm0EEES2_S2_");
extern "C" void agiru_unlinked_175() { Unlinked("agiru::LibraryXMLRead_Codeunit::VerifyAttributeAbsenceInSubtree(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_176() asm("_ZN5agiru24LibraryGraphMgt_Codeunit15CreateTargetURLENS_4TextILm0EEEiS2_");
extern "C" void agiru_unlinked_176() { Unlinked("agiru::LibraryGraphMgt_Codeunit::CreateTargetURL(agiru::Text<0ul>, int, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_177() asm("_ZN5agiru24LibraryGraphMgt_Codeunit17AddPropertytoJSONENS_4TextILm0EEES2_NS_7VariantE");
extern "C" void agiru_unlinked_177() { Unlinked("agiru::LibraryGraphMgt_Codeunit::AddPropertytoJSON(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Variant)"); }
extern "C" void agiru_unlinked_178() asm("_ZN5agiru24LibraryGraphMgt_Codeunit19GetObjectIDFromJSONENS_4TextILm0EEES2_RS2_");
extern "C" void agiru_unlinked_178() { Unlinked("agiru::LibraryGraphMgt_Codeunit::GetObjectIDFromJSON(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_179() asm("_ZN5agiru24LibraryGraphMgt_Codeunit20AddComplexTypetoJSONENS_4TextILm0EEES2_S2_");
extern "C" void agiru_unlinked_179() { Unlinked("agiru::LibraryGraphMgt_Codeunit::AddComplexTypetoJSON(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_180() asm("_ZN5agiru24LibraryGraphMgt_Codeunit21EnsureWebServiceExistENS_4TextILm240EEEi");
extern "C" void agiru_unlinked_180() { Unlinked("agiru::LibraryGraphMgt_Codeunit::EnsureWebServiceExist(agiru::Text<240ul>, int)"); }
extern "C" void agiru_unlinked_181() asm("_ZN5agiru24LibraryGraphMgt_Codeunit36PostToWebServiceAndCheckResponseCodeENS_4TextILm0EEES2_RS2_i");
extern "C" void agiru_unlinked_181() { Unlinked("agiru::LibraryGraphMgt_Codeunit::PostToWebServiceAndCheckResponseCode(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>&, int)"); }
extern "C" void agiru_unlinked_182() asm("_ZN5agiru24LibraryGraphMgt_Codeunit37GetFromWebServiceAndCheckResponseCodeERNS_4TextILm0EEES2_i");
extern "C" void agiru_unlinked_182() { Unlinked("agiru::LibraryGraphMgt_Codeunit::GetFromWebServiceAndCheckResponseCode(agiru::Text<0ul>&, agiru::Text<0ul>, int)"); }
extern "C" void agiru_unlinked_183() asm("_ZN5agiru24LibraryGraphMgt_Codeunit37PatchToWebServiceAndCheckResponseCodeENS_4TextILm0EEES2_RS2_i");
extern "C" void agiru_unlinked_183() { Unlinked("agiru::LibraryGraphMgt_Codeunit::PatchToWebServiceAndCheckResponseCode(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>&, int)"); }
extern "C" void agiru_unlinked_184() asm("_ZN5agiru27LibraryPermissions_Codeunit10CreateUserERNS_8platform10User_TableENS_4TextILm50EEEb");
extern "C" void agiru_unlinked_184() { Unlinked("agiru::LibraryPermissions_Codeunit::CreateUser(agiru::platform::User_Table&, agiru::Text<50ul>, bool)"); }
extern "C" void agiru_unlinked_185() asm("_ZN5agiru27LibraryPermissions_Codeunit13AddPermissionENS_4CodeILm20EEENS_6OptionIvEEi");
extern "C" void agiru_unlinked_185() { Unlinked("agiru::LibraryPermissions_Codeunit::AddPermission(agiru::Code<20ul>, agiru::Option<void>, int)"); }
extern "C" void agiru_unlinked_186() asm("_ZN5agiru27LibraryPermissions_Codeunit13AddUserToPlanENS_4GuidES1_");
extern "C" void agiru_unlinked_186() { Unlinked("agiru::LibraryPermissions_Codeunit::AddUserToPlan(agiru::Guid, agiru::Guid)"); }
extern "C" void agiru_unlinked_187() asm("_ZN5agiru27LibraryPermissions_Codeunit17CreateWindowsUserERNS_8platform10User_TableENS_4CodeILm50EEE");
extern "C" void agiru_unlinked_187() { Unlinked("agiru::LibraryPermissions_Codeunit::CreateWindowsUser(agiru::platform::User_Table&, agiru::Code<50ul>)"); }
extern "C" void agiru_unlinked_188() asm("_ZN5agiru27LibraryPermissions_Codeunit18CreateUserWithNameENS_4TextILm50EEE");
extern "C" void agiru_unlinked_188() { Unlinked("agiru::LibraryPermissions_Codeunit::CreateUserWithName(agiru::Text<50ul>)"); }
extern "C" void agiru_unlinked_189() asm("_ZN5agiru27LibraryPermissions_Codeunit19AddTenantPermissionENS_4GuidENS_4CodeILm20EEENS_6OptionIvEEi");
extern "C" void agiru_unlinked_189() { Unlinked("agiru::LibraryPermissions_Codeunit::AddTenantPermission(agiru::Guid, agiru::Code<20ul>, agiru::Option<void>, int)"); }
extern "C" void agiru_unlinked_190() asm("_ZN5agiru27LibraryPermissions_Codeunit19CreatePermissionSetERNS_6absent19TenantPermissionSetENS_4CodeILm20EEE");
extern "C" void agiru_unlinked_190() { Unlinked("agiru::LibraryPermissions_Codeunit::CreatePermissionSet(agiru::absent::TenantPermissionSet&, agiru::Code<20ul>)"); }
extern "C" void agiru_unlinked_191() asm("_ZN5agiru27LibraryPermissions_Codeunit25CreatePermissionSetInPlanENS_4CodeILm20EEENS_4GuidE");
extern "C" void agiru_unlinked_191() { Unlinked("agiru::LibraryPermissions_Codeunit::CreatePermissionSetInPlan(agiru::Code<20ul>, agiru::Guid)"); }
extern "C" void agiru_unlinked_192() asm("_ZN5agiru27LibraryPermissions_Codeunit25CreateTenantPermissionSetERNS_6absent19TenantPermissionSetENS_4CodeILm20EEENS_4GuidE");
extern "C" void agiru_unlinked_192() { Unlinked("agiru::LibraryPermissions_Codeunit::CreateTenantPermissionSet(agiru::absent::TenantPermissionSet&, agiru::Code<20ul>, agiru::Guid)"); }
extern "C" void agiru_unlinked_193() asm("_ZN5agiru27LibraryPermissions_Codeunit27CreateWindowsUserSecurityIDENS_4CodeILm50EEE");
extern "C" void agiru_unlinked_193() { Unlinked("agiru::LibraryPermissions_Codeunit::CreateWindowsUserSecurityID(agiru::Code<50ul>)"); }
extern "C" void agiru_unlinked_194() asm("_ZN5agiru27LibraryPermissions_Codeunit28SetTestTenantEnvironmentTypeEb");
extern "C" void agiru_unlinked_194() { Unlinked("agiru::LibraryPermissions_Codeunit::SetTestTenantEnvironmentType(bool)"); }
extern "C" void agiru_unlinked_195() asm("_ZN5agiru27LibraryPermissions_Codeunit32SetTestabilitySoftwareAsAServiceEb");
extern "C" void agiru_unlinked_195() { Unlinked("agiru::LibraryPermissions_Codeunit::SetTestabilitySoftwareAsAService(bool)"); }
extern "C" void agiru_unlinked_196() asm("_ZN5agiru27LibraryPermissions_Codeunit9GetMyUserERNS_8platform10User_TableE");
extern "C" void agiru_unlinked_196() { Unlinked("agiru::LibraryPermissions_Codeunit::GetMyUser(agiru::platform::User_Table&)"); }
extern "C" void agiru_unlinked_197() asm("_ZN5agiru27O365SyncManagement_Codeunit11IsO365SetupEb");
extern "C" void agiru_unlinked_197() { Unlinked("agiru::O365SyncManagement_Codeunit::IsO365Setup(bool)"); }
extern "C" void agiru_unlinked_198() asm("_ZN5agiru27O365SyncManagement_Codeunit12ShowProgressENS_4TextILm0EEE");
extern "C" void agiru_unlinked_198() { Unlinked("agiru::O365SyncManagement_Codeunit::ShowProgress(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_199() asm("_ZN5agiru27O365SyncManagement_Codeunit13CloseProgressEv");
extern "C" void agiru_unlinked_199() { Unlinked("agiru::O365SyncManagement_Codeunit::CloseProgress()"); }
extern "C" void agiru_unlinked_200() asm("_ZN5agiru27O365SyncManagement_Codeunit13TraceCategoryEv");
extern "C" void agiru_unlinked_200() { Unlinked("agiru::O365SyncManagement_Codeunit::TraceCategory()"); }
extern "C" void agiru_unlinked_201() asm("_ZN5agiru27O365SyncManagement_Codeunit17LogActivityFailedENS_7VariantENS_4CodeILm50EEENS_4TextILm0EEES5_");
extern "C" void agiru_unlinked_201() { Unlinked("agiru::O365SyncManagement_Codeunit::LogActivityFailed(agiru::Variant, agiru::Code<50ul>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_202() asm("_ZN5agiru27O365SyncManagement_Codeunit20SyncExchangeContactsENS_3CRM7Outlook18ExchangeSync_TableEb");
extern "C" void agiru_unlinked_202() { Unlinked("agiru::O365SyncManagement_Codeunit::SyncExchangeContacts(agiru::CRM::Outlook::ExchangeSync_Table, bool)"); }
extern "C" void agiru_unlinked_203() asm("_ZN5agiru27O365SyncManagement_Codeunit24CreateExchangeConnectionERNS_3CRM7Outlook18ExchangeSync_TableE");
extern "C" void agiru_unlinked_203() { Unlinked("agiru::O365SyncManagement_Codeunit::CreateExchangeConnection(agiru::CRM::Outlook::ExchangeSync_Table&)"); }
extern "C" void agiru_unlinked_204() asm("_ZN5agiru28ERMPESourceTestMock_Codeunit15GetTempBlobListERNS_6System9Utilities21TempBlobList_CodeunitE");
extern "C" void agiru_unlinked_204() { Unlinked("agiru::ERMPESourceTestMock_Codeunit::GetTempBlobList(agiru::System::Utilities::TempBlobList_Codeunit&)"); }
extern "C" void agiru_unlinked_205() asm("_ZN5agiru28ERMPESourceTestMock_Codeunit15SetTempBlobListENS_6System9Utilities21TempBlobList_CodeunitE");
extern "C" void agiru_unlinked_205() { Unlinked("agiru::ERMPESourceTestMock_Codeunit::SetTempBlobList(agiru::System::Utilities::TempBlobList_Codeunit)"); }
extern "C" void agiru_unlinked_206() asm("_ZN5agiru28ERMPESourceTestMock_Codeunit17ClearTempBlobListEv");
extern "C" void agiru_unlinked_206() { Unlinked("agiru::ERMPESourceTestMock_Codeunit::ClearTempBlobList()"); }
extern "C" void agiru_unlinked_207() asm("_ZN5agiru28PostingCodeunitMock_Codeunit14GetLogFileNameEv");
extern "C" void agiru_unlinked_207() { Unlinked("agiru::PostingCodeunitMock_Codeunit::GetLogFileName()"); }
extern "C" void agiru_unlinked_208() asm("_ZN5agiru28PostingCodeunitMock_Codeunit26RunWithActiveErrorHandlingERNS_6System9Utilities18ErrorMessage_TableEb");
extern "C" void agiru_unlinked_208() { Unlinked("agiru::PostingCodeunitMock_Codeunit::RunWithActiveErrorHandling(agiru::System::Utilities::ErrorMessage_Table&, bool)"); }
extern "C" void agiru_unlinked_209() asm("_ZN5agiru28PostingCodeunitMock_Codeunit6TryRunERNS_6System9Utilities18ErrorMessage_TableE");
extern "C" void agiru_unlinked_209() { Unlinked("agiru::PostingCodeunitMock_Codeunit::TryRun(agiru::System::Utilities::ErrorMessage_Table&)"); }
extern "C" void agiru_unlinked_210() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit10InitializeENS_4TextILm0EEES2_");
extern "C" void agiru_unlinked_210() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::Initialize(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_211() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit11GetNodeListENS_4TextILm0EEERNS_6dotnet11XmlNodeListE");
extern "C" void agiru_unlinked_211() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::GetNodeList(agiru::Text<0ul>, agiru::dotnet::XmlNodeList&)"); }
extern "C" void agiru_unlinked_212() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit13InitializeXmlENS_6System9Utilities17TempBlob_CodeunitENS_4TextILm0EEE");
extern "C" void agiru_unlinked_212() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::InitializeXml(agiru::System::Utilities::TempBlob_Codeunit, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_213() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit14GetNodeByXPathENS_4TextILm0EEERNS_6dotnet7XmlNodeE");
extern "C" void agiru_unlinked_213() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::GetNodeByXPath(agiru::Text<0ul>, agiru::dotnet::XmlNode&)"); }
extern "C" void agiru_unlinked_214() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit18InitializeWithBlobENS_6System9Utilities17TempBlob_CodeunitENS_4TextILm0EEE");
extern "C" void agiru_unlinked_214() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::InitializeWithBlob(agiru::System::Utilities::TempBlob_Codeunit, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_215() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit20GetElementInCurrNodeENS_6dotnet7XmlNodeENS_4TextILm0EEERS2_");
extern "C" void agiru_unlinked_215() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::GetElementInCurrNode(agiru::dotnet::XmlNode, agiru::Text<0ul>, agiru::dotnet::XmlNode&)"); }
extern "C" void agiru_unlinked_216() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit21GetNodeListInCurrNodeENS_6dotnet7XmlNodeENS_4TextILm0EEERNS1_11XmlNodeListE");
extern "C" void agiru_unlinked_216() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::GetNodeListInCurrNode(agiru::dotnet::XmlNode, agiru::Text<0ul>, agiru::dotnet::XmlNodeList&)"); }
extern "C" void agiru_unlinked_217() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit22AddAdditionalNamespaceENS_4TextILm0EEES2_");
extern "C" void agiru_unlinked_217() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::AddAdditionalNamespace(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_218() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit22VerifyNodeCountByXPathENS_4TextILm0EEEi");
extern "C" void agiru_unlinked_218() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::VerifyNodeCountByXPath(agiru::Text<0ul>, int)"); }
extern "C" void agiru_unlinked_219() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit22VerifyNodeValueByXPathENS_4TextILm0EEES2_");
extern "C" void agiru_unlinked_219() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::VerifyNodeValueByXPath(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_220() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit23VerifyAttributeFromNodeENS_6dotnet7XmlNodeENS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_220() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::VerifyAttributeFromNode(agiru::dotnet::XmlNode, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_221() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit24SetDefaultNamespaceUsageEb");
extern "C" void agiru_unlinked_221() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::SetDefaultNamespaceUsage(bool)"); }
extern "C" void agiru_unlinked_222() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit26VerifyNodeAbsenceInSubtreeENS_4TextILm0EEES2_");
extern "C" void agiru_unlinked_222() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::VerifyNodeAbsenceInSubtree(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_223() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit27GetNodeByElementNameByIndexENS_4TextILm0EEERNS_6dotnet7XmlNodeEi");
extern "C" void agiru_unlinked_223() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::GetNodeByElementNameByIndex(agiru::Text<0ul>, agiru::dotnet::XmlNode&, int)"); }
extern "C" void agiru_unlinked_224() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit29VerifyNodeValueFromParentNodeENS_6dotnet7XmlNodeENS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_224() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::VerifyNodeValueFromParentNode(agiru::dotnet::XmlNode, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_225() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit30VerifyAttributeAbsenceFromNodeENS_6dotnet7XmlNodeENS_4TextILm0EEE");
extern "C" void agiru_unlinked_225() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::VerifyAttributeAbsenceFromNode(agiru::dotnet::XmlNode, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_226() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit31VerifyNodeValueByXPathWithIndexENS_4TextILm0EEES2_i");
extern "C" void agiru_unlinked_226() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::VerifyNodeValueByXPathWithIndex(agiru::Text<0ul>, agiru::Text<0ul>, int)"); }
extern "C" void agiru_unlinked_227() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit31VerifyOptionalAttributeFromNodeENS_6dotnet7XmlNodeENS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_227() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::VerifyOptionalAttributeFromNode(agiru::dotnet::XmlNode, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_228() asm("_ZN5agiru30LibraryXPathXMLReader_Codeunit32GetNodeInnerTextByXPathWithIndexENS_4TextILm0EEEi");
extern "C" void agiru_unlinked_228() { Unlinked("agiru::LibraryXPathXMLReader_Codeunit::GetNodeInnerTextByXPathWithIndex(agiru::Text<0ul>, int)"); }
extern "C" void agiru_unlinked_229() asm("_ZN5agiru30TableRelationTypeMismatch_Page6OnInitEv");
extern "C" void agiru_unlinked_229() { Unlinked("agiru::TableRelationTypeMismatch_Page::OnInit()"); }
extern "C" void agiru_unlinked_230() asm("_ZN5agiru38MockOnPostNotificationRequest_Codeunit13SetReturnTypeENS_4TextILm0EEE");
extern "C" void agiru_unlinked_230() { Unlinked("agiru::MockOnPostNotificationRequest_Codeunit::SetReturnType(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_231() asm("_ZN5agiru3CRM7Outlook16ContactSync_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_231() { Unlinked("agiru::CRM::Outlook::ContactSync_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_232() asm("_ZN5agiru3CRM7Outlook22ExchangeSyncSetup_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_232() { Unlinked("agiru::CRM::Outlook::ExchangeSyncSetup_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_233() asm("_ZN5agiru3CRM7Outlook29OutlookSynchTypeConv_Codeunit17TextToOptionValueENS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_233() { Unlinked("agiru::CRM::Outlook::OutlookSynchTypeConv_Codeunit::TextToOptionValue(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_234() asm("_ZN5agiru3CRM7Outlook29OutlookSynchTypeConv_Codeunit22EvaluateTextToFieldRefENS_4TextILm0EEERNS_8FieldRefEb");
extern "C" void agiru_unlinked_234() { Unlinked("agiru::CRM::Outlook::OutlookSynchTypeConv_Codeunit::EvaluateTextToFieldRef(agiru::Text<0ul>, agiru::FieldRef&, bool)"); }
extern "C" void agiru_unlinked_235() asm("_ZN5agiru3CRM7Outlook30OfficeDocumentHandler_Codeunit18HandleSalesCommandENS_5Sales8Customer14Customer_TableENS1_24OfficeAddInContext_TableE");
extern "C" void agiru_unlinked_235() { Unlinked("agiru::CRM::Outlook::OfficeDocumentHandler_Codeunit::HandleSalesCommand(agiru::Sales::Customer::Customer_Table, agiru::CRM::Outlook::OfficeAddInContext_Table)"); }
extern "C" void agiru_unlinked_236() asm("_ZN5agiru3CRM7Outlook30OfficeDocumentHandler_Codeunit21HandlePurchaseCommandENS_9Purchases6Vendor12Vendor_TableENS1_24OfficeAddInContext_TableE");
extern "C" void agiru_unlinked_236() { Unlinked("agiru::CRM::Outlook::OfficeDocumentHandler_Codeunit::HandlePurchaseCommand(agiru::Purchases::Vendor::Vendor_Table, agiru::CRM::Outlook::OfficeAddInContext_Table)"); }
extern "C" void agiru_unlinked_237() asm("_ZN5agiru3CRM7Outlook30OfficeDocumentHandler_Codeunit21ShowDocumentSelectionEii");
extern "C" void agiru_unlinked_237() { Unlinked("agiru::CRM::Outlook::OfficeDocumentHandler_Codeunit::ShowDocumentSelection(int, int)"); }
extern "C" void agiru_unlinked_238() asm("_ZN5agiru3CRM7Outlook30OfficeDocumentHandler_Codeunit22OpenIndividualDocumentENS1_24OfficeAddInContext_TableENS1_29OfficeDocumentSelection_TableE");
extern "C" void agiru_unlinked_238() { Unlinked("agiru::CRM::Outlook::OfficeDocumentHandler_Codeunit::OpenIndividualDocument(agiru::CRM::Outlook::OfficeAddInContext_Table, agiru::CRM::Outlook::OfficeDocumentSelection_Table)"); }
extern "C" void agiru_unlinked_239() asm("_ZN5agiru3CRM8Analysis21OpportunityChart_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_239() { Unlinked("agiru::CRM::Analysis::OpportunityChart_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_240() asm("_ZN5agiru5Sales7Posting26SalesBatchPostMgt_Codeunit12SetParameterENS_4EnumINS_10Foundation15BatchProcessing30BatchPostingParameterType_EnumEEENS_7VariantE");
extern "C" void agiru_unlinked_240() { Unlinked("agiru::Sales::Posting::SalesBatchPostMgt_Codeunit::SetParameter(agiru::Enum<agiru::Foundation::BatchProcessing::BatchPostingParameterType_Enum>, agiru::Variant)"); }
extern "C" void agiru_unlinked_241() asm("_ZN5agiru5Sales7Posting26SalesBatchPostMgt_Codeunit17SetBatchProcessorENS_10Foundation15BatchProcessing27BatchProcessingMgt_CodeunitE");
extern "C" void agiru_unlinked_241() { Unlinked("agiru::Sales::Posting::SalesBatchPostMgt_Codeunit::SetBatchProcessor(agiru::Foundation::BatchProcessing::BatchProcessingMgt_Codeunit)"); }
extern "C" void agiru_unlinked_242() asm("_ZN5agiru5Sales7Posting26SalesBatchPostMgt_Codeunit8RunBatchERNS0_8Document17SalesHeader_TableEbNS_4DateEbbbb");
extern "C" void agiru_unlinked_242() { Unlinked("agiru::Sales::Posting::SalesBatchPostMgt_Codeunit::RunBatch(agiru::Sales::Document::SalesHeader_Table&, bool, agiru::Date, bool, bool, bool, bool)"); }
extern "C" void agiru_unlinked_243() asm("_ZN5agiru5Sales7Posting26SalesBatchPostMgt_Codeunit9RunWithUIERNS0_8Document17SalesHeader_TableEiNS_4TextILm0EEE");
extern "C" void agiru_unlinked_243() { Unlinked("agiru::Sales::Posting::SalesBatchPostMgt_Codeunit::RunWithUI(agiru::Sales::Document::SalesHeader_Table&, int, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_244() asm("_ZN5agiru5Sales8Analysis23SalesPipelineChart_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_244() { Unlinked("agiru::Sales::Analysis::SalesPipelineChart_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_245() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_245() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_246() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page11OnActionAllEv");
extern "C" void agiru_unlinked_246() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnActionAll()"); }
extern "C" void agiru_unlinked_247() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page12OnEnabledAllEv");
extern "C" void agiru_unlinked_247() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnEnabledAll()"); }
extern "C" void agiru_unlinked_248() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page17OnActionDayPeriodEv");
extern "C" void agiru_unlinked_248() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnActionDayPeriod()"); }
extern "C" void agiru_unlinked_249() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page18OnActionWeekPeriodEv");
extern "C" void agiru_unlinked_249() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnActionWeekPeriod()"); }
extern "C" void agiru_unlinked_250() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page18OnActionYearPeriodEv");
extern "C" void agiru_unlinked_250() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnActionYearPeriod()"); }
extern "C" void agiru_unlinked_251() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page18OnEnabledDayPeriodEv");
extern "C" void agiru_unlinked_251() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnEnabledDayPeriod()"); }
extern "C" void agiru_unlinked_252() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page19OnActionMonthPeriodEv");
extern "C" void agiru_unlinked_252() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnActionMonthPeriod()"); }
extern "C" void agiru_unlinked_253() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page19OnEnabledWeekPeriodEv");
extern "C" void agiru_unlinked_253() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnEnabledWeekPeriod()"); }
extern "C" void agiru_unlinked_254() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page19OnEnabledYearPeriodEv");
extern "C" void agiru_unlinked_254() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnEnabledYearPeriod()"); }
extern "C" void agiru_unlinked_255() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_255() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_256() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page20OnEnabledMonthPeriodEv");
extern "C" void agiru_unlinked_256() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnEnabledMonthPeriod()"); }
extern "C" void agiru_unlinked_257() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page21OnActionQuarterPeriodEv");
extern "C" void agiru_unlinked_257() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnActionQuarterPeriod()"); }
extern "C" void agiru_unlinked_258() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page22OnEnabledQuarterPeriodEv");
extern "C" void agiru_unlinked_258() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnEnabledQuarterPeriod()"); }
extern "C" void agiru_unlinked_259() asm("_ZN5agiru5Sales8Analysis27AgedAccReceivableChart_Page24OnActionChartInformationEv");
extern "C" void agiru_unlinked_259() { Unlinked("agiru::Sales::Analysis::AgedAccReceivableChart_Page::OnActionChartInformation()"); }
extern "C" void agiru_unlinked_260() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_260() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_261() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page11OnActionDayEv");
extern "C" void agiru_unlinked_261() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionDay()"); }
extern "C" void agiru_unlinked_262() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page12OnActionWeekEv");
extern "C" void agiru_unlinked_262() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionWeek()"); }
extern "C" void agiru_unlinked_263() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page12OnActionYearEv");
extern "C" void agiru_unlinked_263() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionYear()"); }
extern "C" void agiru_unlinked_264() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page12OnEnabledDayEv");
extern "C" void agiru_unlinked_264() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledDay()"); }
extern "C" void agiru_unlinked_265() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page13OnActionMonthEv");
extern "C" void agiru_unlinked_265() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionMonth()"); }
extern "C" void agiru_unlinked_266() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page13OnActionSetupEv");
extern "C" void agiru_unlinked_266() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionSetup()"); }
extern "C" void agiru_unlinked_267() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page13OnEnabledWeekEv");
extern "C" void agiru_unlinked_267() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledWeek()"); }
extern "C" void agiru_unlinked_268() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page13OnEnabledYearEv");
extern "C" void agiru_unlinked_268() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledYear()"); }
extern "C" void agiru_unlinked_269() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page14OnActionAmountEv");
extern "C" void agiru_unlinked_269() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionAmount()"); }
extern "C" void agiru_unlinked_270() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page14OnEnabledMonthEv");
extern "C" void agiru_unlinked_270() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledMonth()"); }
extern "C" void agiru_unlinked_271() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page15OnActionQuarterEv");
extern "C" void agiru_unlinked_271() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionQuarter()"); }
extern "C" void agiru_unlinked_272() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page15OnEnabledAmountEv");
extern "C" void agiru_unlinked_272() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledAmount()"); }
extern "C" void agiru_unlinked_273() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page16OnEnabledQuarterEv");
extern "C" void agiru_unlinked_273() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledQuarter()"); }
extern "C" void agiru_unlinked_274() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page17OnActionAllOrdersEv");
extern "C" void agiru_unlinked_274() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionAllOrders()"); }
extern "C" void agiru_unlinked_275() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page18OnActionNoofOrdersEv");
extern "C" void agiru_unlinked_275() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionNoofOrders()"); }
extern "C" void agiru_unlinked_276() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page18OnEnabledAllOrdersEv");
extern "C" void agiru_unlinked_276() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledAllOrders()"); }
extern "C" void agiru_unlinked_277() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page19OnActionStackedAreaEv");
extern "C" void agiru_unlinked_277() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionStackedArea()"); }
extern "C" void agiru_unlinked_278() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page19OnEnabledNoofOrdersEv");
extern "C" void agiru_unlinked_278() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledNoofOrders()"); }
extern "C" void agiru_unlinked_279() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page20OnEnabledStackedAreaEv");
extern "C" void agiru_unlinked_279() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledStackedArea()"); }
extern "C" void agiru_unlinked_280() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page21OnActionDelayedOrdersEv");
extern "C" void agiru_unlinked_280() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionDelayedOrders()"); }
extern "C" void agiru_unlinked_281() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page21OnActionStackedColumnEv");
extern "C" void agiru_unlinked_281() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionStackedColumn()"); }
extern "C" void agiru_unlinked_282() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page22OnActionStackedAreaPctEv");
extern "C" void agiru_unlinked_282() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionStackedAreaPct()"); }
extern "C" void agiru_unlinked_283() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page22OnEnabledDelayedOrdersEv");
extern "C" void agiru_unlinked_283() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledDelayedOrders()"); }
extern "C" void agiru_unlinked_284() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page22OnEnabledStackedColumnEv");
extern "C" void agiru_unlinked_284() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledStackedColumn()"); }
extern "C" void agiru_unlinked_285() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page23OnEnabledStackedAreaPctEv");
extern "C" void agiru_unlinked_285() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledStackedAreaPct()"); }
extern "C" void agiru_unlinked_286() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page24OnActionOrdersUntilTodayEv");
extern "C" void agiru_unlinked_286() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionOrdersUntilToday()"); }
extern "C" void agiru_unlinked_287() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page24OnActionStackedColumnPctEv");
extern "C" void agiru_unlinked_287() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnActionStackedColumnPct()"); }
extern "C" void agiru_unlinked_288() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page25OnEnabledOrdersUntilTodayEv");
extern "C" void agiru_unlinked_288() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledOrdersUntilToday()"); }
extern "C" void agiru_unlinked_289() asm("_ZN5agiru5Sales8Analysis29TrailingSalesOrdersChart_Page25OnEnabledStackedColumnPctEv");
extern "C" void agiru_unlinked_289() { Unlinked("agiru::Sales::Analysis::TrailingSalesOrdersChart_Page::OnEnabledStackedColumnPct()"); }
extern "C" void agiru_unlinked_290() asm("_ZN5agiru5Sales8Reminder23AdditionalFeeChart_Page10UpdateDataEv");
extern "C" void agiru_unlinked_290() { Unlinked("agiru::Sales::Reminder::AdditionalFeeChart_Page::UpdateData()"); }
extern "C" void agiru_unlinked_291() asm("_ZN5agiru5Sales8Reminder23AdditionalFeeChart_Page11SetViewModeENS1_19ReminderLevel_TableEbb");
extern "C" void agiru_unlinked_291() { Unlinked("agiru::Sales::Reminder::AdditionalFeeChart_Page::SetViewMode(agiru::Sales::Reminder::ReminderLevel_Table, bool, bool)"); }
extern "C" void agiru_unlinked_292() asm("_ZN5agiru5Sales8Reminder23AdditionalFeeChart_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_292() { Unlinked("agiru::Sales::Reminder::AdditionalFeeChart_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_293() asm("_ZN5agiru5Sales8Reminder23AdditionalFeeChart_Page18OnValidateCurrencyEv");
extern "C" void agiru_unlinked_293() { Unlinked("agiru::Sales::Reminder::AdditionalFeeChart_Page::OnValidateCurrency()"); }
extern "C" void agiru_unlinked_294() asm("_ZN5agiru5Sales8Reminder23AdditionalFeeChart_Page22OnVisibleChargePerLineEv");
extern "C" void agiru_unlinked_294() { Unlinked("agiru::Sales::Reminder::AdditionalFeeChart_Page::OnVisibleChargePerLine()"); }
extern "C" void agiru_unlinked_295() asm("_ZN5agiru5Sales8Reminder23AdditionalFeeChart_Page23OnValidateChargePerLineEv");
extern "C" void agiru_unlinked_295() { Unlinked("agiru::Sales::Reminder::AdditionalFeeChart_Page::OnValidateChargePerLine()"); }
extern "C" void agiru_unlinked_296() asm("_ZN5agiru5Sales8Reminder23AdditionalFeeChart_Page28OnValidateMaxRemainingAmountEv");
extern "C" void agiru_unlinked_296() { Unlinked("agiru::Sales::Reminder::AdditionalFeeChart_Page::OnValidateMaxRemainingAmount()"); }
extern "C" void agiru_unlinked_297() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit14EditInOneDriveENS_6absent16ReportLayoutListE");
extern "C" void agiru_unlinked_297() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::EditInOneDrive(agiru::absent::ReportLayoutList)"); }
extern "C" void agiru_unlinked_298() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit14OpenInOneDriveENS_6absent16ReportLayoutListE");
extern "C" void agiru_unlinked_298() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::OpenInOneDrive(agiru::absent::ReportLayoutList)"); }
extern "C" void agiru_unlinked_299() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit14ShowInfoDialogENS_6absent16ReportLayoutListE");
extern "C" void agiru_unlinked_299() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::ShowInfoDialog(agiru::absent::ReportLayoutList)"); }
extern "C" void agiru_unlinked_300() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit14ValidateLayoutENS_6absent16ReportLayoutListE");
extern "C" void agiru_unlinked_300() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::ValidateLayout(agiru::absent::ReportLayoutList)"); }
extern "C" void agiru_unlinked_301() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit15InsertNewLayoutEiNS_4TextILm250EEES4_NS_6OptionIvEEbbNS_4EnumIvEES8_RiRNS3_ILm0EEE");
extern "C" void agiru_unlinked_301() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::InsertNewLayout(int, agiru::Text<250ul>, agiru::Text<250ul>, agiru::Option<void>, bool, bool, agiru::Enum<void>, agiru::Enum<void>, int&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_302() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit15RunCustomReportENS_6absent16ReportLayoutListE");
extern "C" void agiru_unlinked_302() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::RunCustomReport(agiru::absent::ReportLayoutList)"); }
extern "C" void agiru_unlinked_303() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit16EditReportLayoutENS_6absent16ReportLayoutListERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_303() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::EditReportLayout(agiru::absent::ReportLayoutList, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_304() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit17ShareWithOneDriveENS_6absent16ReportLayoutListE");
extern "C" void agiru_unlinked_304() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::ShareWithOneDrive(agiru::absent::ReportLayoutList)"); }
extern "C" void agiru_unlinked_305() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit18DeleteReportLayoutENS_6absent18TenantReportLayoutE");
extern "C" void agiru_unlinked_305() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::DeleteReportLayout(agiru::absent::TenantReportLayout)"); }
extern "C" void agiru_unlinked_306() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit18ExportReportLayoutENS_6absent16ReportLayoutListEb");
extern "C" void agiru_unlinked_306() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::ExportReportLayout(agiru::absent::ReportLayoutList, bool)"); }
extern "C" void agiru_unlinked_307() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit18ExportReportSchemaENS_6absent16ReportLayoutListENS_4TextILm0EEEb");
extern "C" void agiru_unlinked_307() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::ExportReportSchema(agiru::absent::ReportLayoutList, agiru::Text<0ul>, bool)"); }
extern "C" void agiru_unlinked_308() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit18SetSelectedCompanyENS_4TextILm0EEE");
extern "C" void agiru_unlinked_308() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::SetSelectedCompany(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_309() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit20SetLayoutStatusBatchERNS_6absent16ReportLayoutListENS_4EnumIvEE");
extern "C" void agiru_unlinked_309() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::SetLayoutStatusBatch(agiru::absent::ReportLayoutList&, agiru::Enum<void>)"); }
extern "C" void agiru_unlinked_310() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit21CreateNewReportLayoutENS_6absent16ReportLayoutListENS_4EnumIvEERiRNS_4TextILm0EEE");
extern "C" void agiru_unlinked_310() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::CreateNewReportLayout(agiru::absent::ReportLayoutList, agiru::Enum<void>, int&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_311() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit31GetDefaultReportLayoutSelectionEiRNS_6absent16ReportLayoutListE");
extern "C" void agiru_unlinked_311() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::GetDefaultReportLayoutSelection(int, agiru::absent::ReportLayoutList&)"); }
extern "C" void agiru_unlinked_312() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit31SetDefaultReportLayoutSelectionENS_6absent16ReportLayoutListEb");
extern "C" void agiru_unlinked_312() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::SetDefaultReportLayoutSelection(agiru::absent::ReportLayoutList, bool)"); }
extern "C" void agiru_unlinked_313() asm("_ZN5agiru6Shared6Report26ReportLayoutsImpl_Codeunit35ConfirmDeleteDefaultLayoutSelectionENS_6absent16ReportLayoutListENS3_27TenantReportLayoutSelectionE");
extern "C" void agiru_unlinked_313() { Unlinked("agiru::Shared::Report::ReportLayoutsImpl_Codeunit::ConfirmDeleteDefaultLayoutSelection(agiru::absent::ReportLayoutList, agiru::absent::TenantReportLayoutSelection)"); }
extern "C" void agiru_unlinked_314() asm("_ZN5agiru6System10Automation36WorkflowWebhookNotification_Codeunit10InitializeEii");
extern "C" void agiru_unlinked_314() { Unlinked("agiru::System::Automation::WorkflowWebhookNotification_Codeunit::Initialize(int, int)"); }
extern "C" void agiru_unlinked_315() asm("_ZN5agiru6System10Automation36WorkflowWebhookNotification_Codeunit11ShouldRetryEiNS_4TextILm0EEE");
extern "C" void agiru_unlinked_315() { Unlinked("agiru::System::Automation::WorkflowWebhookNotification_Codeunit::ShouldRetry(int, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_316() asm("_ZN5agiru6System10Automation36WorkflowWebhookNotification_Codeunit16SendNotificationENS_4GuidES3_NS_4TextILm0EEES5_");
extern "C" void agiru_unlinked_316() { Unlinked("agiru::System::Automation::WorkflowWebhookNotification_Codeunit::SendNotification(agiru::Guid, agiru::Guid, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_317() asm("_ZN5agiru6System10Automation36WorkflowWebhookNotification_Codeunit17StartNotificationENS_4GuidE");
extern "C" void agiru_unlinked_317() { Unlinked("agiru::System::Automation::WorkflowWebhookNotification_Codeunit::StartNotification(agiru::Guid)"); }
extern "C" void agiru_unlinked_318() asm("_ZN5agiru6System10Reflection28RecordSelectionImpl_Codeunit21GetRecordsFromTableIdEiNS_7AlArrayINS_4TextILm0EEELm0EEERNS1_27RecordSelectionBuffer_TableE");
extern "C" void agiru_unlinked_318() { Unlinked("agiru::System::Reflection::RecordSelectionImpl_Codeunit::GetRecordsFromTableId(int, agiru::AlArray<agiru::Text<0ul>, 0ul>, agiru::System::Reflection::RecordSelectionBuffer_Table&)"); }
extern "C" void agiru_unlinked_319() asm("_ZN5agiru6System10Reflection28RecordSelectionImpl_Codeunit4OpenEiiRNS1_27RecordSelectionBuffer_TableE");
extern "C" void agiru_unlinked_319() { Unlinked("agiru::System::Reflection::RecordSelectionImpl_Codeunit::Open(int, int, agiru::System::Reflection::RecordSelectionBuffer_Table&)"); }
extern "C" void agiru_unlinked_320() asm("_ZN5agiru6System10Reflection28RecordSelectionImpl_Codeunit6ToTextEiNS_4GuidE");
extern "C" void agiru_unlinked_320() { Unlinked("agiru::System::Reflection::RecordSelectionImpl_Codeunit::ToText(int, agiru::Guid)"); }
extern "C" void agiru_unlinked_321() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit10GetTimeOutEv");
extern "C" void agiru_unlinked_321() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::GetTimeOut()"); }
extern "C" void agiru_unlinked_322() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit10InitializeENS_14ImplementationINS1_27HttpClientHandler_InterfaceEEE");
extern "C" void agiru_unlinked_322() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Initialize(agiru::Implementation<agiru::System::RestClient::HttpClientHandler_Interface>)"); }
extern "C" void agiru_unlinked_323() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit10InitializeENS_14ImplementationINS1_27HttpClientHandler_InterfaceEEENS3_INS1_28HttpAuthentication_InterfaceEEE");
extern "C" void agiru_unlinked_323() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Initialize(agiru::Implementation<agiru::System::RestClient::HttpClientHandler_Interface>, agiru::Implementation<agiru::System::RestClient::HttpAuthentication_Interface>)"); }
extern "C" void agiru_unlinked_324() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit10InitializeENS_14ImplementationINS1_28HttpAuthentication_InterfaceEEE");
extern "C" void agiru_unlinked_324() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Initialize(agiru::Implementation<agiru::System::RestClient::HttpAuthentication_Interface>)"); }
extern "C" void agiru_unlinked_325() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit10InitializeEv");
extern "C" void agiru_unlinked_325() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Initialize()"); }
extern "C" void agiru_unlinked_326() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit10PostAsJsonENS_4TextILm0EEENS_9JsonTokenE");
extern "C" void agiru_unlinked_326() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::PostAsJson(agiru::Text<0ul>, agiru::JsonToken)"); }
extern "C" void agiru_unlinked_327() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit10SetTimeOutENS_8DurationE");
extern "C" void agiru_unlinked_327() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::SetTimeOut(agiru::Duration)"); }
extern "C" void agiru_unlinked_328() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit11PatchAsJsonENS_4TextILm0EEENS_9JsonTokenE");
extern "C" void agiru_unlinked_328() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::PatchAsJson(agiru::Text<0ul>, agiru::JsonToken)"); }
extern "C" void agiru_unlinked_329() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit14AddCertificateENS_4TextILm0EEE");
extern "C" void agiru_unlinked_329() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::AddCertificate(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_330() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit14AddCertificateENS_4TextILm0EEENS_10SecretTextE");
extern "C" void agiru_unlinked_330() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::AddCertificate(agiru::Text<0ul>, agiru::SecretText)"); }
extern "C" void agiru_unlinked_331() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit14GetBaseAddressEv");
extern "C" void agiru_unlinked_331() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::GetBaseAddress()"); }
extern "C" void agiru_unlinked_332() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit14SetBaseAddressENS_4TextILm0EEE");
extern "C" void agiru_unlinked_332() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::SetBaseAddress(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_333() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit18SetUserAgentHeaderENS_4TextILm0EEE");
extern "C" void agiru_unlinked_333() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::SetUserAgentHeader(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_334() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit21SetUseResponseCookiesEb");
extern "C" void agiru_unlinked_334() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::SetUseResponseCookies(bool)"); }
extern "C" void agiru_unlinked_335() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit22SetAuthorizationHeaderENS_10SecretTextE");
extern "C" void agiru_unlinked_335() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::SetAuthorizationHeader(agiru::SecretText)"); }
extern "C" void agiru_unlinked_336() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit23SetDefaultRequestHeaderENS_4TextILm0EEENS_10SecretTextE");
extern "C" void agiru_unlinked_336() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::SetDefaultRequestHeader(agiru::Text<0ul>, agiru::SecretText)"); }
extern "C" void agiru_unlinked_337() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit23SetDefaultRequestHeaderENS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_337() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::SetDefaultRequestHeader(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_338() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit33SetUseServerCertificateValidationEb");
extern "C" void agiru_unlinked_338() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::SetUseServerCertificateValidation(bool)"); }
extern "C" void agiru_unlinked_339() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit4SendENS_4EnumINS1_15HttpMethod_EnumEEENS_4TextILm0EEE");
extern "C" void agiru_unlinked_339() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Send(agiru::Enum<agiru::System::RestClient::HttpMethod_Enum>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_340() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit4SendENS_4EnumINS1_15HttpMethod_EnumEEENS_4TextILm0EEENS1_20HttpContent_CodeunitE");
extern "C" void agiru_unlinked_340() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Send(agiru::Enum<agiru::System::RestClient::HttpMethod_Enum>, agiru::Text<0ul>, agiru::System::RestClient::HttpContent_Codeunit)"); }
extern "C" void agiru_unlinked_341() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit4SendERNS1_27HttpRequestMessage_CodeunitE");
extern "C" void agiru_unlinked_341() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Send(agiru::System::RestClient::HttpRequestMessage_Codeunit&)"); }
extern "C" void agiru_unlinked_342() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit6CreateENS_14ImplementationINS1_27HttpClientHandler_InterfaceEEE");
extern "C" void agiru_unlinked_342() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Create(agiru::Implementation<agiru::System::RestClient::HttpClientHandler_Interface>)"); }
extern "C" void agiru_unlinked_343() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit6CreateENS_14ImplementationINS1_27HttpClientHandler_InterfaceEEENS3_INS1_28HttpAuthentication_InterfaceEEE");
extern "C" void agiru_unlinked_343() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Create(agiru::Implementation<agiru::System::RestClient::HttpClientHandler_Interface>, agiru::Implementation<agiru::System::RestClient::HttpAuthentication_Interface>)"); }
extern "C" void agiru_unlinked_344() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit6CreateENS_14ImplementationINS1_28HttpAuthentication_InterfaceEEE");
extern "C" void agiru_unlinked_344() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Create(agiru::Implementation<agiru::System::RestClient::HttpAuthentication_Interface>)"); }
extern "C" void agiru_unlinked_345() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit6CreateEv");
extern "C" void agiru_unlinked_345() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::Create()"); }
extern "C" void agiru_unlinked_346() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit9GetAsJsonENS_4TextILm0EEE");
extern "C" void agiru_unlinked_346() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::GetAsJson(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_347() asm("_ZN5agiru6System10RestClient23RestClientImpl_Codeunit9PutAsJsonENS_4TextILm0EEENS_9JsonTokenE");
extern "C" void agiru_unlinked_347() { Unlinked("agiru::System::RestClient::RestClientImpl_Codeunit::PutAsJson(agiru::Text<0ul>, agiru::JsonToken)"); }
extern "C" void agiru_unlinked_348() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit12IsValidTableEi");
extern "C" void agiru_unlinked_348() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::IsValidTable(int)"); }
extern "C" void agiru_unlinked_349() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit12OpenUserCardENS_4CodeILm250EEE");
extern "C" void agiru_unlinked_349() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::OpenUserCard(agiru::Code<250ul>)"); }
extern "C" void agiru_unlinked_350() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit13EnableMonitorEb");
extern "C" void agiru_unlinked_350() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::EnableMonitor(bool)"); }
extern "C" void agiru_unlinked_351() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit13GetSetupTableERNS1_26FieldMonitoringSetup_TableE");
extern "C" void agiru_unlinked_351() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::GetSetupTable(agiru::System::Diagnostics::FieldMonitoringSetup_Table&)"); }
extern "C" void agiru_unlinked_352() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit13SetSetupTableENS_4TextILm50EEENS_4GuidENS3_ILm250EEENS_4EnumINS0_5Email19EmailConnector_EnumEEE");
extern "C" void agiru_unlinked_352() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::SetSetupTable(agiru::Text<50ul>, agiru::Guid, agiru::Text<250ul>, agiru::Enum<agiru::System::Email::EmailConnector_Enum>)"); }
extern "C" void agiru_unlinked_353() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit14DisableMonitorEv");
extern "C" void agiru_unlinked_353() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::DisableMonitor()"); }
extern "C" void agiru_unlinked_354() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit15ValidateTableNoEi");
extern "C" void agiru_unlinked_354() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::ValidateTableNo(int)"); }
extern "C" void agiru_unlinked_355() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit17AddMonitoredFieldEiib");
extern "C" void agiru_unlinked_355() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::AddMonitoredField(int, int, bool)"); }
extern "C" void agiru_unlinked_356() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit20AddValidTablesFilterERNS_8platform23AllObjWithCaption_TableE");
extern "C" void agiru_unlinked_356() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::AddValidTablesFilter(agiru::platform::AllObjWithCaption_Table&)"); }
extern "C" void agiru_unlinked_357() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit20GetNotificationCountEv");
extern "C" void agiru_unlinked_357() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::GetNotificationCount()"); }
extern "C" void agiru_unlinked_358() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit22AddAllowedFieldFiltersERNS_8platform5FieldE");
extern "C" void agiru_unlinked_358() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::AddAllowedFieldFilters(agiru::platform::Field&)"); }
extern "C" void agiru_unlinked_359() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit23ValidateTableAndFieldNoEii");
extern "C" void agiru_unlinked_359() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::ValidateTableAndFieldNo(int, int)"); }
extern "C" void agiru_unlinked_360() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit23ValidateUserPermissionsENS_4CodeILm50EEERb");
extern "C" void agiru_unlinked_360() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::ValidateUserPermissions(agiru::Code<50ul>, bool&)"); }
extern "C" void agiru_unlinked_361() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit25DeleteChangeLogSetupTableEii");
extern "C" void agiru_unlinked_361() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::DeleteChangeLogSetupTable(int, int)"); }
extern "C" void agiru_unlinked_362() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit25ImportFieldsBySensitivityEbbb");
extern "C" void agiru_unlinked_362() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::ImportFieldsBySensitivity(bool, bool, bool)"); }
extern "C" void agiru_unlinked_363() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit25InsertChangeLogSetupTableEi");
extern "C" void agiru_unlinked_363() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::InsertChangeLogSetupTable(int)"); }
extern "C" void agiru_unlinked_364() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit25ShowPromotionNotificationEv");
extern "C" void agiru_unlinked_364() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::ShowPromotionNotification()"); }
extern "C" void agiru_unlinked_365() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit29CheckUserHasValidContactEmailENS_4CodeILm50EEE");
extern "C" void agiru_unlinked_365() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::CheckUserHasValidContactEmail(agiru::Code<50ul>)"); }
extern "C" void agiru_unlinked_366() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit29OpenDataSensitivityFilterPageEv");
extern "C" void agiru_unlinked_366() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::OpenDataSensitivityFilterPage()"); }
extern "C" void agiru_unlinked_367() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit33ExcludeMonitorTablesFromChangeLogERNS_8platform23AllObjWithCaption_TableE");
extern "C" void agiru_unlinked_367() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::ExcludeMonitorTablesFromChangeLog(agiru::platform::AllObjWithCaption_Table&)"); }
extern "C" void agiru_unlinked_368() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit38GetChangeLogHiddenTablesNotificationIdEv");
extern "C" void agiru_unlinked_368() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::GetChangeLogHiddenTablesNotificationId()"); }
extern "C" void agiru_unlinked_369() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit38GetPromoteMonitorFeatureNotificationIdEv");
extern "C" void agiru_unlinked_369() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::GetPromoteMonitorFeatureNotificationId()"); }
extern "C" void agiru_unlinked_370() asm("_ZN5agiru6System11Diagnostics30MonitorSensitiveField_Codeunit44ShowPromoteMonitorSensitiveFieldNotificationEv");
extern "C" void agiru_unlinked_370() { Unlinked("agiru::System::Diagnostics::MonitorSensitiveField_Codeunit::ShowPromoteMonitorSensitiveFieldNotification()"); }
extern "C" void agiru_unlinked_371() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_371() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_372() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page11OnNewRecordEb");
extern "C" void agiru_unlinked_372() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnNewRecord(bool)"); }
extern "C" void agiru_unlinked_373() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page14OnInsertRecordEb");
extern "C" void agiru_unlinked_373() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnInsertRecord(bool)"); }
extern "C" void agiru_unlinked_374() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page14OnModifyRecordEv");
extern "C" void agiru_unlinked_374() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnModifyRecord()"); }
extern "C" void agiru_unlinked_375() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page17OnEditableGeneralEv");
extern "C" void agiru_unlinked_375() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnEditableGeneral()"); }
extern "C" void agiru_unlinked_376() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_376() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_377() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page22OnValidateEnabledFieldEv");
extern "C" void agiru_unlinked_377() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnValidateEnabledField()"); }
extern "C" void agiru_unlinked_378() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page24OnEditableProfileIdFieldEv");
extern "C" void agiru_unlinked_378() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnEditableProfileIdField()"); }
extern "C" void agiru_unlinked_379() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page24OnValidateProfileIdFieldEv");
extern "C" void agiru_unlinked_379() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnValidateProfileIdField()"); }
extern "C" void agiru_unlinked_380() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page25OnActionCopyProfileActionEv");
extern "C" void agiru_unlinked_380() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnActionCopyProfileAction()"); }
extern "C" void agiru_unlinked_381() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page25OnEditableProfileSettingsEv");
extern "C" void agiru_unlinked_381() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnEditableProfileSettings()"); }
extern "C" void agiru_unlinked_382() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page25OnLookupRoleCenterIdFieldERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_382() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnLookupRoleCenterIdField(agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_383() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page27OnActionCustomizeRoleActionEv");
extern "C" void agiru_unlinked_383() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnActionCustomizeRoleAction()"); }
extern "C" void agiru_unlinked_384() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page27OnValidateRoleCenterIdFieldEv");
extern "C" void agiru_unlinked_384() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnValidateRoleCenterIdField()"); }
extern "C" void agiru_unlinked_385() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page28OnEnabledCustomizeRoleActionEv");
extern "C" void agiru_unlinked_385() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnEnabledCustomizeRoleAction()"); }
extern "C" void agiru_unlinked_386() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page28OnVisibleCustomizeRoleActionEv");
extern "C" void agiru_unlinked_386() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnVisibleCustomizeRoleAction()"); }
extern "C" void agiru_unlinked_387() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page32OnValidateDefaultRoleCenterFieldEv");
extern "C" void agiru_unlinked_387() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnValidateDefaultRoleCenterField()"); }
extern "C" void agiru_unlinked_388() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page34OnActionClearCustomizedPagesActionEv");
extern "C" void agiru_unlinked_388() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnActionClearCustomizedPagesAction()"); }
extern "C" void agiru_unlinked_389() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page35OnEnabledClearCustomizedPagesActionEv");
extern "C" void agiru_unlinked_389() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnEnabledClearCustomizedPagesAction()"); }
extern "C" void agiru_unlinked_390() asm("_ZN5agiru6System11Environment13Configuration16ProfileCard_Page6OnInitEv");
extern "C" void agiru_unlinked_390() { Unlinked("agiru::System::Environment::Configuration::ProfileCard_Page::OnInit()"); }
extern "C" void agiru_unlinked_391() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_391() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_392() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page14OnVisibleFilesEv");
extern "C" void agiru_unlinked_392() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnVisibleFiles()"); }
extern "C" void agiru_unlinked_393() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page16OnQueryClosePageENS_6ActionE");
extern "C" void agiru_unlinked_393() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnQueryClosePage(agiru::Action)"); }
extern "C" void agiru_unlinked_394() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page17OnVisibleSecurityEv");
extern "C" void agiru_unlinked_394() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnVisibleSecurity()"); }
extern "C" void agiru_unlinked_395() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page18OnAssistEditRegionEv");
extern "C" void agiru_unlinked_395() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnAssistEditRegion()"); }
extern "C" void agiru_unlinked_396() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page19OnAssistEditCompanyEv");
extern "C" void agiru_unlinked_396() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnAssistEditCompany()"); }
extern "C" void agiru_unlinked_397() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_397() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_398() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page20OnAssistEditTimeZoneEv");
extern "C" void agiru_unlinked_398() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnAssistEditTimeZone()"); }
extern "C" void agiru_unlinked_399() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page22OnVisibleLastLoginInfoEv");
extern "C" void agiru_unlinked_399() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnVisibleLastLoginInfo()"); }
extern "C" void agiru_unlinked_400() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page24OnAssistEditLanguageNameEv");
extern "C" void agiru_unlinked_400() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnAssistEditLanguageName()"); }
extern "C" void agiru_unlinked_401() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page26OnAssistEditUserRoleCenterEv");
extern "C" void agiru_unlinked_401() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnAssistEditUserRoleCenter()"); }
extern "C" void agiru_unlinked_402() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page27OnVisibleMyNotificationsLblEv");
extern "C" void agiru_unlinked_402() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnVisibleMyNotificationsLbl()"); }
extern "C" void agiru_unlinked_403() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page29OnDrillDownMyNotificationsLblEv");
extern "C" void agiru_unlinked_403() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnDrillDownMyNotificationsLbl()"); }
extern "C" void agiru_unlinked_404() asm("_ZN5agiru6System11Environment13Configuration17UserSettings_Page31OnDrillDownOpenOneDriveBCFolderEv");
extern "C" void agiru_unlinked_404() { Unlinked("agiru::System::Environment::Configuration::UserSettings_Page::OnDrillDownOpenOneDriveBCFolder()"); }
extern "C" void agiru_unlinked_405() asm("_ZN5agiru6System11Environment13Configuration21AdvancedSettings_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_405() { Unlinked("agiru::System::Environment::Configuration::AdvancedSettings_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_406() asm("_ZN5agiru6System11Environment13Configuration24UserPersonalization_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_406() { Unlinked("agiru::System::Environment::Configuration::UserPersonalization_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_407() asm("_ZN5agiru6System11Environment13Configuration24UserPersonalization_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_407() { Unlinked("agiru::System::Environment::Configuration::UserPersonalization_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_408() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit13ProfileLookupERNS2_18UserSettings_TableE");
extern "C" void agiru_unlinked_408() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::ProfileLookup(agiru::System::Environment::Configuration::UserSettings_Table&)"); }
extern "C" void agiru_unlinked_409() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit14GetProfileNameENS_6OptionINS_7options18OptionSystemTenantEEENS_4GuidENS_4CodeILm30EEE");
extern "C" void agiru_unlinked_409() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::GetProfileName(agiru::Option<agiru::options::OptionSystemTenant>, agiru::Guid, agiru::Code<30ul>)"); }
extern "C" void agiru_unlinked_410() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit15GetUserSettingsENS_4GuidERNS2_18UserSettings_TableE");
extern "C" void agiru_unlinked_410() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::GetUserSettings(agiru::Guid, agiru::System::Environment::Configuration::UserSettings_Table&)"); }
extern "C" void agiru_unlinked_411() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit16PopulateProfilesERNS_8platform16AllProfile_TableE");
extern "C" void agiru_unlinked_411() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::PopulateProfiles(agiru::platform::AllProfile_Table&)"); }
extern "C" void agiru_unlinked_412() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit17HideExternalUsersERNS_8platform25UserPersonalization_TableE");
extern "C" void agiru_unlinked_412() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::HideExternalUsers(agiru::platform::UserPersonalization_Table&)"); }
extern "C" void agiru_unlinked_413() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit18EnableTeachingTipsENS_4GuidE");
extern "C" void agiru_unlinked_413() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::EnableTeachingTips(agiru::Guid)"); }
extern "C" void agiru_unlinked_414() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit18UpdateUserSettingsENS2_18UserSettings_TableE");
extern "C" void agiru_unlinked_414() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::UpdateUserSettings(agiru::System::Environment::Configuration::UserSettings_Table)"); }
extern "C" void agiru_unlinked_415() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit19DisableTeachingTipsENS_4GuidE");
extern "C" void agiru_unlinked_415() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::DisableTeachingTips(agiru::Guid)"); }
extern "C" void agiru_unlinked_416() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit19GetAllUsersSettingsERNS2_18UserSettings_TableE");
extern "C" void agiru_unlinked_416() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::GetAllUsersSettings(agiru::System::Environment::Configuration::UserSettings_Table&)"); }
extern "C" void agiru_unlinked_417() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit21EnableLegacyActionBarENS_4GuidE");
extern "C" void agiru_unlinked_417() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::EnableLegacyActionBar(agiru::Guid)"); }
extern "C" void agiru_unlinked_418() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit22DisableLegacyActionBarENS_4GuidE");
extern "C" void agiru_unlinked_418() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::DisableLegacyActionBar(agiru::Guid)"); }
extern "C" void agiru_unlinked_419() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit26GetAllowedCompaniesForUserENS_4GuidERNS_8platform13Company_TableE");
extern "C" void agiru_unlinked_419() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::GetAllowedCompaniesForUser(agiru::Guid, agiru::platform::Company_Table&)"); }
extern "C" void agiru_unlinked_420() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit31HideUsersDependingOnPermissionsERNS_8platform25UserPersonalization_TableE");
extern "C" void agiru_unlinked_420() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::HideUsersDependingOnPermissions(agiru::platform::UserPersonalization_Table&)"); }
extern "C" void agiru_unlinked_421() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit33GetAllowedCompaniesForCurrentUserERNS_8platform13Company_TableE");
extern "C" void agiru_unlinked_421() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::GetAllowedCompaniesForCurrentUser(agiru::platform::Company_Table&)"); }
extern "C" void agiru_unlinked_422() asm("_ZN5agiru6System11Environment13Configuration25UserSettingsImpl_Codeunit9GetPageIdEv");
extern "C" void agiru_unlinked_422() { Unlinked("agiru::System::Environment::Configuration::UserSettingsImpl_Codeunit::GetPageId()"); }
extern "C" void agiru_unlinked_423() asm("_ZN5agiru6System11Environment13Configuration28MS365LicenseSetupWizard_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_423() { Unlinked("agiru::System::Environment::Configuration::MS365LicenseSetupWizard_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_424() asm("_ZN5agiru6System11Environment13Configuration28TextSearchLanguageSetup_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_424() { Unlinked("agiru::System::Environment::Configuration::TextSearchLanguageSetup_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_425() asm("_ZN5agiru6System11Environment13Configuration29FeatureDataUpdateMgt_Codeunit17FeatureKeyMatchesENS2_29FeatureDataUpdateStatus_TableENS_4EnumINS2_20FeatureToUpdate_EnumEEE");
extern "C" void agiru_unlinked_425() { Unlinked("agiru::System::Environment::Configuration::FeatureDataUpdateMgt_Codeunit::FeatureKeyMatches(agiru::System::Environment::Configuration::FeatureDataUpdateStatus_Table, agiru::Enum<agiru::System::Environment::Configuration::FeatureToUpdate_Enum>)"); }
extern "C" void agiru_unlinked_426() asm("_ZN5agiru6System11Environment13Configuration29FeatureDataUpdateMgt_Codeunit7LogTaskENS2_29FeatureDataUpdateStatus_TableENS_4TextILm0EEENS_8DateTimeE");
extern "C" void agiru_unlinked_426() { Unlinked("agiru::System::Environment::Configuration::FeatureDataUpdateMgt_Codeunit::LogTask(agiru::System::Environment::Configuration::FeatureDataUpdateStatus_Table, agiru::Text<0ul>, agiru::DateTime)"); }
extern "C" void agiru_unlinked_427() asm("_ZN5agiru6System11Environment15ImportData_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_427() { Unlinked("agiru::System::Environment::ImportData_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_428() asm("_ZN5agiru6System11Environment15ImportData_Page16OnQueryClosePageENS_6ActionE");
extern "C" void agiru_unlinked_428() { Unlinked("agiru::System::Environment::ImportData_Page::OnQueryClosePage(agiru::Action)"); }
extern "C" void agiru_unlinked_429() asm("_ZN5agiru6System11Environment15ImportData_Page6OnInitEv");
extern "C" void agiru_unlinked_429() { Unlinked("agiru::System::Environment::ImportData_Page::OnInit()"); }
extern "C" void agiru_unlinked_430() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_430() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_431() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page11OnActionDayEv");
extern "C" void agiru_unlinked_431() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionDay()"); }
extern "C" void agiru_unlinked_432() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page12OnActionWeekEv");
extern "C" void agiru_unlinked_432() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionWeek()"); }
extern "C" void agiru_unlinked_433() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page12OnActionYearEv");
extern "C" void agiru_unlinked_433() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionYear()"); }
extern "C" void agiru_unlinked_434() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page13OnActionMonthEv");
extern "C" void agiru_unlinked_434() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionMonth()"); }
extern "C" void agiru_unlinked_435() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page15OnActionQuarterEv");
extern "C" void agiru_unlinked_435() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionQuarter()"); }
extern "C" void agiru_unlinked_436() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page17OnActionNextChartEv");
extern "C" void agiru_unlinked_436() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionNextChart()"); }
extern "C" void agiru_unlinked_437() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page18OnActionNextPeriodEv");
extern "C" void agiru_unlinked_437() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionNextPeriod()"); }
extern "C" void agiru_unlinked_438() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page19OnActionSelectChartEv");
extern "C" void agiru_unlinked_438() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionSelectChart()"); }
extern "C" void agiru_unlinked_439() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page19OnEnabledNextPeriodEv");
extern "C" void agiru_unlinked_439() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnEnabledNextPeriod()"); }
extern "C" void agiru_unlinked_440() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page21OnActionPreviousChartEv");
extern "C" void agiru_unlinked_440() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionPreviousChart()"); }
extern "C" void agiru_unlinked_441() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page22OnActionPreviousPeriodEv");
extern "C" void agiru_unlinked_441() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionPreviousPeriod()"); }
extern "C" void agiru_unlinked_442() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page23OnEnabledPreviousPeriodEv");
extern "C" void agiru_unlinked_442() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnEnabledPreviousPeriod()"); }
extern "C" void agiru_unlinked_443() asm("_ZN5agiru6System11Environment24HelpAndChartWrapper_Page24OnActionChartInformationEv");
extern "C" void agiru_unlinked_443() { Unlinked("agiru::System::Environment::HelpAndChartWrapper_Page::OnActionChartInformation()"); }
extern "C" void agiru_unlinked_444() asm("_ZN5agiru6System11Environment25HybridDeployment_Codeunit14RunReplicationERNS_4TextILm0EEEi");
extern "C" void agiru_unlinked_444() { Unlinked("agiru::System::Environment::HybridDeployment_Codeunit::RunReplication(agiru::Text<0ul>&, int)"); }
extern "C" void agiru_unlinked_445() asm("_ZN5agiru6System11Environment25HybridDeployment_Codeunit17EnableReplicationENS_4TextILm0EEES4_S4_");
extern "C" void agiru_unlinked_445() { Unlinked("agiru::System::Environment::HybridDeployment_Codeunit::EnableReplication(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_446() asm("_ZN5agiru6System11Environment25HybridDeployment_Codeunit18DisableReplicationEv");
extern "C" void agiru_unlinked_446() { Unlinked("agiru::System::Environment::HybridDeployment_Codeunit::DisableReplication()"); }
extern "C" void agiru_unlinked_447() asm("_ZN5agiru6System11Environment25HybridDeployment_Codeunit21VerifyCanStartUpgradeENS_4TextILm0EEE");
extern "C" void agiru_unlinked_447() { Unlinked("agiru::System::Environment::HybridDeployment_Codeunit::VerifyCanStartUpgrade(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_448() asm("_ZN5agiru6System11Environment25HybridDeployment_Codeunit22SetReplicationScheduleENS_4TextILm0EEES4_NS_4TimeEb");
extern "C" void agiru_unlinked_448() { Unlinked("agiru::System::Environment::HybridDeployment_Codeunit::SetReplicationSchedule(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Time, bool)"); }
extern "C" void agiru_unlinked_449() asm("_ZN5agiru6System11Environment25HybridDeployment_Codeunit23GetReplicationRunStatusENS_4TextILm0EEERS4_S5_");
extern "C" void agiru_unlinked_449() { Unlinked("agiru::System::Environment::HybridDeployment_Codeunit::GetReplicationRunStatus(agiru::Text<0ul>, agiru::Text<0ul>&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_450() asm("_ZN5agiru6System11Environment25HybridDeployment_Codeunit24CreateIntegrationRuntimeERNS_4TextILm0EEES5_");
extern "C" void agiru_unlinked_450() { Unlinked("agiru::System::Environment::HybridDeployment_Codeunit::CreateIntegrationRuntime(agiru::Text<0ul>&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_451() asm("_ZN5agiru6System11Environment25HybridDeployment_Codeunit25GetIntegrationRuntimeKeysERNS_4TextILm0EEES5_");
extern "C" void agiru_unlinked_451() { Unlinked("agiru::System::Environment::HybridDeployment_Codeunit::GetIntegrationRuntimeKeys(agiru::Text<0ul>&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_452() asm("_ZN5agiru6System11Environment25HybridDeployment_Codeunit32RegenerateIntegrationRuntimeKeysERNS_4TextILm0EEES5_");
extern "C" void agiru_unlinked_452() { Unlinked("agiru::System::Environment::HybridDeployment_Codeunit::RegenerateIntegrationRuntimeKeys(agiru::Text<0ul>&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_453() asm("_ZN5agiru6System11Environment30TenantInformationImpl_Codeunit11GetTenantIdEv");
extern "C" void agiru_unlinked_453() { Unlinked("agiru::System::Environment::TenantInformationImpl_Codeunit::GetTenantId()"); }
extern "C" void agiru_unlinked_454() asm("_ZN5agiru6System11Environment30TenantInformationImpl_Codeunit20GetTenantDisplayNameEv");
extern "C" void agiru_unlinked_454() { Unlinked("agiru::System::Environment::TenantInformationImpl_Codeunit::GetTenantDisplayName()"); }
extern "C" void agiru_unlinked_455() asm("_ZN5agiru6System11Integration10Sharepoint36SharePointOperationResponse_Codeunit14GetDiagnosticsEv");
extern "C" void agiru_unlinked_455() { Unlinked("agiru::System::Integration::Sharepoint::SharePointOperationResponse_Codeunit::GetDiagnostics()"); }
extern "C" void agiru_unlinked_456() asm("_ZN5agiru6System11Integration10Sharepoint36SharePointOperationResponse_Codeunit15GetResultAsTextERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_456() { Unlinked("agiru::System::Integration::Sharepoint::SharePointOperationResponse_Codeunit::GetResultAsText(agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_457() asm("_ZN5agiru6System11Integration10Sharepoint36SharePointOperationResponse_Codeunit15SetHttpResponseENS_19HttpResponseMessageE");
extern "C" void agiru_unlinked_457() { Unlinked("agiru::System::Integration::Sharepoint::SharePointOperationResponse_Codeunit::SetHttpResponse(agiru::HttpResponseMessage)"); }
extern "C" void agiru_unlinked_458() asm("_ZN5agiru6System11Integration10Sharepoint36SharePointOperationResponse_Codeunit15SetHttpResponseENS_4TextILm0EEENS_11HttpHeadersEibS5_");
extern "C" void agiru_unlinked_458() { Unlinked("agiru::System::Integration::Sharepoint::SharePointOperationResponse_Codeunit::SetHttpResponse(agiru::Text<0ul>, agiru::HttpHeaders, int, bool, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_459() asm("_ZN5agiru6System11Integration10Sharepoint36SharePointOperationResponse_Codeunit17GetResultAsStreamERNS_8InStreamE");
extern "C" void agiru_unlinked_459() { Unlinked("agiru::System::Integration::Sharepoint::SharePointOperationResponse_Codeunit::GetResultAsStream(agiru::InStream&)"); }
extern "C" void agiru_unlinked_460() asm("_ZN5agiru6System11Integration21ODataSetupWizard_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_460() { Unlinked("agiru::System::Integration::ODataSetupWizard_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_461() asm("_ZN5agiru6System11Integration21ODataSetupWizard_Page16OnQueryClosePageENS_6ActionE");
extern "C" void agiru_unlinked_461() { Unlinked("agiru::System::Integration::ODataSetupWizard_Page::OnQueryClosePage(agiru::Action)"); }
extern "C" void agiru_unlinked_462() asm("_ZN5agiru6System11Integration21ODataSetupWizard_Page6OnInitEv");
extern "C" void agiru_unlinked_462() { Unlinked("agiru::System::Integration::ODataSetupWizard_Page::OnInit()"); }
extern "C" void agiru_unlinked_463() asm("_ZN5agiru6System11Integration21ODataUtility_Codeunit15ExternalizeNameENS_4TextILm0EEE");
extern "C" void agiru_unlinked_463() { Unlinked("agiru::System::Integration::ODataUtility_Codeunit::ExternalizeName(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_464() asm("_ZN5agiru6System11Integration21ODataUtility_Codeunit18GenerateSelectTextENS_4TextILm0EEENS_6OptionINS_7options64OptionBlankBlank1Blank2Blank3Blank4CodeunitBlank6Blank_213783174EEERS4_");
extern "C" void agiru_unlinked_464() { Unlinked("agiru::System::Integration::ODataUtility_Codeunit::GenerateSelectText(agiru::Text<0ul>, agiru::Option<agiru::options::OptionBlankBlank1Blank2Blank3Blank4CodeunitBlank6Blank_213783174>, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_465() asm("_ZN5agiru6System11Integration21ODataUtility_Codeunit24CreateMetadataWebRequestERNS1_26HttpWebRequestMgt_CodeunitE");
extern "C" void agiru_unlinked_465() { Unlinked("agiru::System::Integration::ODataUtility_Codeunit::CreateMetadataWebRequest(agiru::System::Integration::HttpWebRequestMgt_Codeunit&)"); }
extern "C" void agiru_unlinked_466() asm("_ZN5agiru6System11Integration21ODataUtility_Codeunit25GenerateODataV3FilterTextENS_4TextILm0EEENS_6OptionINS_7options64OptionBlankBlank1Blank2Blank3Blank4CodeunitBlank6Blank_213783174EEERS4_");
extern "C" void agiru_unlinked_466() { Unlinked("agiru::System::Integration::ODataUtility_Codeunit::GenerateODataV3FilterText(agiru::Text<0ul>, agiru::Option<agiru::options::OptionBlankBlank1Blank2Blank3Blank4CodeunitBlank6Blank_213783174>, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_467() asm("_ZN5agiru6System11Integration21ODataUtility_Codeunit25GenerateODataV4FilterTextENS_4TextILm0EEENS_6OptionINS_7options64OptionBlankBlank1Blank2Blank3Blank4CodeunitBlank6Blank_213783174EEERS4_");
extern "C" void agiru_unlinked_467() { Unlinked("agiru::System::Integration::ODataUtility_Codeunit::GenerateODataV4FilterText(agiru::Text<0ul>, agiru::Option<agiru::options::OptionBlankBlank1Blank2Blank3Blank4CodeunitBlank6Blank_213783174>, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_468() asm("_ZN5agiru6System11Integration21ODataUtility_Codeunit27EditJournalWorksheetInExcelENS_4TextILm240EEENS3_ILm0EEES5_S5_");
extern "C" void agiru_unlinked_468() { Unlinked("agiru::System::Integration::ODataUtility_Codeunit::EditJournalWorksheetInExcel(agiru::Text<240ul>, agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_469() asm("_ZN5agiru6System11Integration21ODataUtility_Codeunit29DownloadODataMetadataDocumentEv");
extern "C" void agiru_unlinked_469() { Unlinked("agiru::System::Integration::ODataUtility_Codeunit::DownloadODataMetadataDocument()"); }
extern "C" void agiru_unlinked_470() asm("_ZN5agiru6System11Integration29ODataColumnChooseSubForm_Page10GetColumnsERNS1_29TenantWebServiceColumns_TableE");
extern "C" void agiru_unlinked_470() { Unlinked("agiru::System::Integration::ODataColumnChooseSubForm_Page::GetColumns(agiru::System::Integration::TenantWebServiceColumns_Table&)"); }
extern "C" void agiru_unlinked_471() asm("_ZN5agiru6System11Integration29ODataColumnChooseSubForm_Page11InitColumnsENS_6OptionINS_7options64OptionBlankBlank1Blank2Blank3Blank4Blank5Blank6Blank7P_937651017EEEiNS3_INS4_64OptionCreateANewDataSetCreateACopyOfAnExistingDataSetE_699518373EEENS_4TextILm0EEESA_");
extern "C" void agiru_unlinked_471() { Unlinked("agiru::System::Integration::ODataColumnChooseSubForm_Page::InitColumns(agiru::Option<agiru::options::OptionBlankBlank1Blank2Blank3Blank4Blank5Blank6Blank7P_937651017>, int, agiru::Option<agiru::options::OptionCreateANewDataSetCreateACopyOfAnExistingDataSetE_699518373>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_472() asm("_ZN5agiru6System11Integration29ODataColumnChooseSubForm_Page23SetCalledForExcelExportERNS_9RecordRefE");
extern "C" void agiru_unlinked_472() { Unlinked("agiru::System::Integration::ODataColumnChooseSubForm_Page::SetCalledForExcelExport(agiru::RecordRef&)"); }
extern "C" void agiru_unlinked_473() asm("_ZN5agiru6System11Integration30VSCodeIntegrationImpl_Codeunit26UpdateDependenciesInVSCodeERNS_6absent20PublishedApplicationE");
extern "C" void agiru_unlinked_473() { Unlinked("agiru::System::Integration::VSCodeIntegrationImpl_Codeunit::UpdateDependenciesInVSCode(agiru::absent::PublishedApplication&)"); }
extern "C" void agiru_unlinked_474() asm("_ZN5agiru6System11Integration30VSCodeIntegrationImpl_Codeunit27OpenExtensionSourceInVSCodeERNS_6absent20PublishedApplicationE");
extern "C" void agiru_unlinked_474() { Unlinked("agiru::System::Integration::VSCodeIntegrationImpl_Codeunit::OpenExtensionSourceInVSCode(agiru::absent::PublishedApplication&)"); }
extern "C" void agiru_unlinked_475() asm("_ZN5agiru6System11Integration30VSCodeIntegrationImpl_Codeunit28UpdateConfigurationsInVSCodeEv");
extern "C" void agiru_unlinked_475() { Unlinked("agiru::System::Integration::VSCodeIntegrationImpl_Codeunit::UpdateConfigurationsInVSCode()"); }
extern "C" void agiru_unlinked_476() asm("_ZN5agiru6System11Integration30VSCodeIntegrationImpl_Codeunit34NavigateToObjectDefinitionInVSCodeENS_6OptionIvEEiNS_4TextILm0EEES6_RNS_6absent18NAVAppInstalledAppE");
extern "C" void agiru_unlinked_476() { Unlinked("agiru::System::Integration::VSCodeIntegrationImpl_Codeunit::NavigateToObjectDefinitionInVSCode(agiru::Option<void>, int, agiru::Text<0ul>, agiru::Text<0ul>, agiru::absent::NAVAppInstalledApp&)"); }
extern "C" void agiru_unlinked_477() asm("_ZN5agiru6System11Integration30VSCodeIntegrationImpl_Codeunit36GetDependenciesAsSerializedJsonArrayERNS_6absent20PublishedApplicationE");
extern "C" void agiru_unlinked_477() { Unlinked("agiru::System::Integration::VSCodeIntegrationImpl_Codeunit::GetDependenciesAsSerializedJsonArray(agiru::absent::PublishedApplication&)"); }
extern "C" void agiru_unlinked_478() asm("_ZN5agiru6System11Integration31PageActionProviderImpl_Codeunit10GetVersionEv");
extern "C" void agiru_unlinked_478() { Unlinked("agiru::System::Integration::PageActionProviderImpl_Codeunit::GetVersion()"); }
extern "C" void agiru_unlinked_479() asm("_ZN5agiru6System11Integration31PageActionProviderImpl_Codeunit29GetCurrentRoleCenterHomeItemsEb");
extern "C" void agiru_unlinked_479() { Unlinked("agiru::System::Integration::PageActionProviderImpl_Codeunit::GetCurrentRoleCenterHomeItems(bool)"); }
extern "C" void agiru_unlinked_480() asm("_ZN5agiru6System11Integration32PageSummaryProviderImpl_Codeunit10GetVersionEv");
extern "C" void agiru_unlinked_480() { Unlinked("agiru::System::Integration::PageSummaryProviderImpl_Codeunit::GetVersion()"); }
extern "C" void agiru_unlinked_481() asm("_ZN5agiru6System11Integration32PageSummaryProviderImpl_Codeunit14GetPageSummaryENS1_27PageSummaryParameters_TableE");
extern "C" void agiru_unlinked_481() { Unlinked("agiru::System::Integration::PageSummaryProviderImpl_Codeunit::GetPageSummary(agiru::System::Integration::PageSummaryParameters_Table)"); }
extern "C" void agiru_unlinked_482() asm("_ZN5agiru6System11Integration32PageSummaryProviderImpl_Codeunit14GetPageSummaryENS_4TextILm0EEE");
extern "C" void agiru_unlinked_482() { Unlinked("agiru::System::Integration::PageSummaryProviderImpl_Codeunit::GetPageSummary(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_483() asm("_ZN5agiru6System11Integration32PageSummaryProviderImpl_Codeunit14GetPageSummaryEiNS_4GuidEbb");
extern "C" void agiru_unlinked_483() { Unlinked("agiru::System::Integration::PageSummaryProviderImpl_Codeunit::GetPageSummary(int, agiru::Guid, bool, bool)"); }
extern "C" void agiru_unlinked_484() asm("_ZN5agiru6System11Integration32PageSummaryProviderImpl_Codeunit14GetPageSummaryEiNS_4TextILm0EEEbb");
extern "C" void agiru_unlinked_484() { Unlinked("agiru::System::Integration::PageSummaryProviderImpl_Codeunit::GetPageSummary(int, agiru::Text<0ul>, bool, bool)"); }
extern "C" void agiru_unlinked_485() asm("_ZN5agiru6System11Integration32PageSummaryProviderImpl_Codeunit20GetPageUrlBySystemIDEiNS_4GuidE");
extern "C" void agiru_unlinked_485() { Unlinked("agiru::System::Integration::PageSummaryProviderImpl_Codeunit::GetPageUrlBySystemID(int, agiru::Guid)"); }
extern "C" void agiru_unlinked_486() asm("_ZN5agiru6System11Integration32PageSummaryProviderImpl_Codeunit37InitializePageSummarySettingsFromJsonENS_4TextILm0EEERNS1_27PageSummaryParameters_TableE");
extern "C" void agiru_unlinked_486() { Unlinked("agiru::System::Integration::PageSummaryProviderImpl_Codeunit::InitializePageSummarySettingsFromJson(agiru::Text<0ul>, agiru::System::Integration::PageSummaryParameters_Table&)"); }
extern "C" void agiru_unlinked_487() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit11LoadRecordsERNS1_25WebServiceAggregate_TableE");
extern "C" void agiru_unlinked_487() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::LoadRecords(agiru::System::Integration::WebServiceAggregate_Table&)"); }
extern "C" void agiru_unlinked_488() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit12VerifyRecordENS1_25WebServiceAggregate_TableE");
extern "C" void agiru_unlinked_488() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::VerifyRecord(agiru::System::Integration::WebServiceAggregate_Table)"); }
extern "C" void agiru_unlinked_489() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit16CreateWebServiceENS_6OptionIvEEiNS_4TextILm0EEEb");
extern "C" void agiru_unlinked_489() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::CreateWebService(agiru::Option<void>, int, agiru::Text<0ul>, bool)"); }
extern "C" void agiru_unlinked_490() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit16DeleteWebServiceERNS1_25WebServiceAggregate_TableE");
extern "C" void agiru_unlinked_490() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::DeleteWebService(agiru::System::Integration::WebServiceAggregate_Table&)"); }
extern "C" void agiru_unlinked_491() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit16GetObjectCaptionENS1_25WebServiceAggregate_TableE");
extern "C" void agiru_unlinked_491() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::GetObjectCaption(agiru::System::Integration::WebServiceAggregate_Table)"); }
extern "C" void agiru_unlinked_492() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit16GetWebServiceUrlENS1_25WebServiceAggregate_TableENS_4EnumINS1_15ClientType_EnumEEE");
extern "C" void agiru_unlinked_492() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::GetWebServiceUrl(agiru::System::Integration::WebServiceAggregate_Table, agiru::Enum<agiru::System::Integration::ClientType_Enum>)"); }
extern "C" void agiru_unlinked_493() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit16InsertWebServiceERNS1_25WebServiceAggregate_TableE");
extern "C" void agiru_unlinked_493() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::InsertWebService(agiru::System::Integration::WebServiceAggregate_Table&)"); }
extern "C" void agiru_unlinked_494() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit16ModifyWebServiceENS1_25WebServiceAggregate_TableES3_");
extern "C" void agiru_unlinked_494() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::ModifyWebService(agiru::System::Integration::WebServiceAggregate_Table, agiru::System::Integration::WebServiceAggregate_Table)"); }
extern "C" void agiru_unlinked_495() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit16RenameWebServiceENS1_25WebServiceAggregate_TableES3_");
extern "C" void agiru_unlinked_495() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::RenameWebService(agiru::System::Integration::WebServiceAggregate_Table, agiru::System::Integration::WebServiceAggregate_Table)"); }
extern "C" void agiru_unlinked_496() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit18IsServiceNameValidENS_4TextILm0EEE");
extern "C" void agiru_unlinked_496() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::IsServiceNameValid(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_497() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit20GetODataFilterClauseENS1_27TenantWebServiceOData_TableE");
extern "C" void agiru_unlinked_497() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::GetODataFilterClause(agiru::System::Integration::TenantWebServiceOData_Table)"); }
extern "C" void agiru_unlinked_498() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit20GetODataSelectClauseENS1_27TenantWebServiceOData_TableE");
extern "C" void agiru_unlinked_498() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::GetODataSelectClause(agiru::System::Integration::TenantWebServiceOData_Table)"); }
extern "C" void agiru_unlinked_499() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit20SetODataFilterClauseERNS1_27TenantWebServiceOData_TableENS_4TextILm0EEE");
extern "C" void agiru_unlinked_499() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::SetODataFilterClause(agiru::System::Integration::TenantWebServiceOData_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_500() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit20SetODataSelectClauseERNS1_27TenantWebServiceOData_TableENS_4TextILm0EEE");
extern "C" void agiru_unlinked_500() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::SetODataSelectClause(agiru::System::Integration::TenantWebServiceOData_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_501() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit21InsertSelectedColumnsERNS_6absent16TenantWebServiceERNS_6dotnet18GenericDictionary2ERNS1_29TenantWebServiceColumns_TableEi");
extern "C" void agiru_unlinked_501() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::InsertSelectedColumns(agiru::absent::TenantWebService&, agiru::dotnet::GenericDictionary2&, agiru::System::Integration::TenantWebServiceColumns_Table&, int)"); }
extern "C" void agiru_unlinked_502() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit22CreateTenantWebServiceENS_6OptionIvEEiNS_4TextILm0EEEb");
extern "C" void agiru_unlinked_502() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::CreateTenantWebService(agiru::Option<void>, int, agiru::Text<0ul>, bool)"); }
extern "C" void agiru_unlinked_503() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit22GetODataV4FilterClauseENS1_27TenantWebServiceOData_TableE");
extern "C" void agiru_unlinked_503() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::GetODataV4FilterClause(agiru::System::Integration::TenantWebServiceOData_Table)"); }
extern "C" void agiru_unlinked_504() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit22SetODataV4FilterClauseERNS1_27TenantWebServiceOData_TableENS_4TextILm0EEE");
extern "C" void agiru_unlinked_504() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::SetODataV4FilterClause(agiru::System::Integration::TenantWebServiceOData_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_505() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit24AssertServiceNameIsValidENS_4TextILm0EEE");
extern "C" void agiru_unlinked_505() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::AssertServiceNameIsValid(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_506() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit25GetTenantWebServiceFilterENS1_28TenantWebServiceFilter_TableE");
extern "C" void agiru_unlinked_506() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::GetTenantWebServiceFilter(agiru::System::Integration::TenantWebServiceFilter_Table)"); }
extern "C" void agiru_unlinked_507() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit25SetTenantWebServiceFilterERNS1_28TenantWebServiceFilter_TableENS_4TextILm0EEE");
extern "C" void agiru_unlinked_507() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::SetTenantWebServiceFilter(agiru::System::Integration::TenantWebServiceFilter_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_508() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit29AssertUniqueUnpublishedObjectENS1_25WebServiceAggregate_TableE");
extern "C" void agiru_unlinked_508() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::AssertUniqueUnpublishedObject(agiru::System::Integration::WebServiceAggregate_Table)"); }
extern "C" void agiru_unlinked_509() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit30RetrieveTenantWebServiceFilterERNS1_28TenantWebServiceFilter_TableE");
extern "C" void agiru_unlinked_509() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::RetrieveTenantWebServiceFilter(agiru::System::Integration::TenantWebServiceFilter_Table&)"); }
extern "C" void agiru_unlinked_510() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit32AssertUniquePublishedServiceNameENS1_25WebServiceAggregate_TableES3_");
extern "C" void agiru_unlinked_510() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::AssertUniquePublishedServiceName(agiru::System::Integration::WebServiceAggregate_Table, agiru::System::Integration::WebServiceAggregate_Table)"); }
extern "C" void agiru_unlinked_511() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit33RemoveUnselectedColumnsFromFilterERNS_6absent16TenantWebServiceEiNS_4TextILm0EEE");
extern "C" void agiru_unlinked_511() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::RemoveUnselectedColumnsFromFilter(agiru::absent::TenantWebService&, int, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_512() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit35CreateTenantWebServiceColumnForPageENS_8RecordIdEii");
extern "C" void agiru_unlinked_512() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::CreateTenantWebServiceColumnForPage(agiru::RecordId, int, int)"); }
extern "C" void agiru_unlinked_513() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit36CreateTenantWebServiceColumnForQueryENS_8RecordIdEiiNS_6dotnet19QueryMetadataReaderE");
extern "C" void agiru_unlinked_513() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::CreateTenantWebServiceColumnForQuery(agiru::RecordId, int, int, agiru::dotnet::QueryMetadataReader)"); }
extern "C" void agiru_unlinked_514() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit37CreateTenantWebServiceColumnsFromTempERNS1_29TenantWebServiceColumns_TableES4_NS_8RecordIdE");
extern "C" void agiru_unlinked_514() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::CreateTenantWebServiceColumnsFromTemp(agiru::System::Integration::TenantWebServiceColumns_Table&, agiru::System::Integration::TenantWebServiceColumns_Table&, agiru::RecordId)"); }
extern "C" void agiru_unlinked_515() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit38LoadRecordsFromTenantWebServiceColumnsERNS_6absent16TenantWebServiceE");
extern "C" void agiru_unlinked_515() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::LoadRecordsFromTenantWebServiceColumns(agiru::absent::TenantWebService&)"); }
extern "C" void agiru_unlinked_516() asm("_ZN5agiru6System11Integration33WebServiceManagementImpl_Codeunit41CreateTenantWebServiceFilterFromRecordRefERNS1_28TenantWebServiceFilter_TableERNS_9RecordRefENS_8RecordIdE");
extern "C" void agiru_unlinked_516() { Unlinked("agiru::System::Integration::WebServiceManagementImpl_Codeunit::CreateTenantWebServiceFilterFromRecordRef(agiru::System::Integration::TenantWebServiceFilter_Table&, agiru::RecordRef&, agiru::RecordId)"); }
extern "C" void agiru_unlinked_517() asm("_ZN5agiru6System11Integration34ExchangeWebServicesClient_Codeunit10ReadBufferERNS_3CRM7Outlook20ExchangeFolder_TableE");
extern "C" void agiru_unlinked_517() { Unlinked("agiru::System::Integration::ExchangeWebServicesClient_Codeunit::ReadBuffer(agiru::CRM::Outlook::ExchangeFolder_Table&)"); }
extern "C" void agiru_unlinked_518() asm("_ZN5agiru6System11Integration34ExchangeWebServicesClient_Codeunit16GetPublicFoldersERNS_3CRM7Outlook20ExchangeFolder_TableE");
extern "C" void agiru_unlinked_518() { Unlinked("agiru::System::Integration::ExchangeWebServicesClient_Codeunit::GetPublicFolders(agiru::CRM::Outlook::ExchangeFolder_Table&)"); }
extern "C" void agiru_unlinked_519() asm("_ZN5agiru6System11Integration4Word18WordTemplates_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_519() { Unlinked("agiru::System::Integration::Word::WordTemplates_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_520() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit10GetFieldNoERNS_4TextILm0EEEii");
extern "C" void agiru_unlinked_520() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GetFieldNo(agiru::Text<0ul>&, int, int)"); }
extern "C" void agiru_unlinked_521() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit10GetTableIdEv");
extern "C" void agiru_unlinked_521() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GetTableId()"); }
extern "C" void agiru_unlinked_522() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit11GetChildrenENS_4CodeILm30EEEi");
extern "C" void agiru_unlinked_522() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GetChildren(agiru::Code<30ul>, int)"); }
extern "C" void agiru_unlinked_523() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit11GetDocumentERNS_8InStreamE");
extern "C" void agiru_unlinked_523() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GetDocument(agiru::InStream&)"); }
extern "C" void agiru_unlinked_524() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit11GetTemplateERNS_8InStreamE");
extern "C" void agiru_unlinked_524() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GetTemplate(agiru::InStream&)"); }
extern "C" void agiru_unlinked_525() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit11RemoveTableENS_4CodeILm30EEEi");
extern "C" void agiru_unlinked_525() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::RemoveTable(agiru::Code<30ul>, int)"); }
extern "C" void agiru_unlinked_526() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit11SelectTableEv");
extern "C" void agiru_unlinked_526() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::SelectTable()"); }
extern "C" void agiru_unlinked_527() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit12GenerateCodeENS_4TextILm249EEENS_10DictionaryINS_4CodeILm5EEEbEE");
extern "C" void agiru_unlinked_527() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GenerateCode(agiru::Text<249ul>, agiru::Dictionary<agiru::Code<5ul>, bool>)"); }
extern "C" void agiru_unlinked_528() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit14GetMergeFieldsERNS_4ListINS_4TextILm0EEEEE");
extern "C" void agiru_unlinked_528() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GetMergeFields(agiru::List<agiru::Text<0ul> >&)"); }
extern "C" void agiru_unlinked_529() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit15AddRelatedTableENS_4CodeILm30EEENS4_ILm5EEEiii");
extern "C" void agiru_unlinked_529() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::AddRelatedTable(agiru::Code<30ul>, agiru::Code<5ul>, int, int, int)"); }
extern "C" void agiru_unlinked_530() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit15AddRelatedTableERNS2_32WordTemplatesRelatedBuffer_TableEibRNS2_23WordTemplateField_TableE");
extern "C" void agiru_unlinked_530() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::AddRelatedTable(agiru::System::Integration::Word::WordTemplatesRelatedBuffer_Table&, int, bool, agiru::System::Integration::Word::WordTemplateField_Table&)"); }
extern "C" void agiru_unlinked_531() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit15GetDocumentSizeEv");
extern "C" void agiru_unlinked_531() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GetDocumentSize()"); }
extern "C" void agiru_unlinked_532() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit15RefreshTreeViewEiRNS2_32WordTemplatesRelatedBuffer_TableE");
extern "C" void agiru_unlinked_532() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::RefreshTreeView(int, agiru::System::Integration::Word::WordTemplatesRelatedBuffer_Table&)"); }
extern "C" void agiru_unlinked_533() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit16AddSelectedTableERNS2_32WordTemplatesRelatedBuffer_TableENS_4CodeILm30EEERNS2_23WordTemplateField_TableE");
extern "C" void agiru_unlinked_533() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::AddSelectedTable(agiru::System::Integration::Word::WordTemplatesRelatedBuffer_Table&, agiru::Code<30ul>, agiru::System::Integration::Word::WordTemplateField_Table&)"); }
extern "C" void agiru_unlinked_534() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit16DownloadDocumentEv");
extern "C" void agiru_unlinked_534() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::DownloadDocument()"); }
extern "C" void agiru_unlinked_535() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit16DownloadTemplateEv");
extern "C" void agiru_unlinked_535() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::DownloadTemplate()"); }
extern "C" void agiru_unlinked_536() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit16EditRelatedTableERNS2_32WordTemplatesRelatedBuffer_TableERNS2_23WordTemplateField_TableE");
extern "C" void agiru_unlinked_536() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::EditRelatedTable(agiru::System::Integration::Word::WordTemplatesRelatedBuffer_Table&, agiru::System::Integration::Word::WordTemplateField_Table&)"); }
extern "C" void agiru_unlinked_537() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit16GetExistingCodesERNS2_31WordTemplatesRelatedTable_TableE");
extern "C" void agiru_unlinked_537() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GetExistingCodes(agiru::System::Integration::Word::WordTemplatesRelatedTable_Table&)"); }
extern "C" void agiru_unlinked_538() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit17AddUnrelatedTableENS_4CodeILm30EEENS4_ILm5EEEiNS_4GuidE");
extern "C" void agiru_unlinked_538() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::AddUnrelatedTable(agiru::Code<30ul>, agiru::Code<5ul>, int, agiru::Guid)"); }
extern "C" void agiru_unlinked_539() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit17AllowedTableExistEi");
extern "C" void agiru_unlinked_539() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::AllowedTableExist(int)"); }
extern "C" void agiru_unlinked_540() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit18InsertWordTemplateERNS2_18WordTemplate_TableE");
extern "C" void agiru_unlinked_540() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::InsertWordTemplate(agiru::System::Integration::Word::WordTemplate_Table&)"); }
extern "C" void agiru_unlinked_541() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit30VerifyRelatedTableCodeIsUniqueENS_4CodeILm30EEENS4_ILm5EEEiRNS2_32WordTemplatesRelatedBuffer_TableE");
extern "C" void agiru_unlinked_541() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::VerifyRelatedTableCodeIsUnique(agiru::Code<30ul>, agiru::Code<5ul>, int, agiru::System::Integration::Word::WordTemplatesRelatedBuffer_Table&)"); }
extern "C" void agiru_unlinked_542() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit4LoadENS_4CodeILm30EEE");
extern "C" void agiru_unlinked_542() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Load(agiru::Code<30ul>)"); }
extern "C" void agiru_unlinked_543() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit4LoadENS_8InStreamE");
extern "C" void agiru_unlinked_543() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Load(agiru::InStream)"); }
extern "C" void agiru_unlinked_544() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit4LoadENS_8InStreamENS_4CodeILm30EEE");
extern "C" void agiru_unlinked_544() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Load(agiru::InStream, agiru::Code<30ul>)"); }
extern "C" void agiru_unlinked_545() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit5MergeENS_10DictionaryINS_4TextILm0EEES6_EEbNS_4EnumINS2_28WordTemplatesSaveFormat_EnumEEE");
extern "C" void agiru_unlinked_545() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Merge(agiru::Dictionary<agiru::Text<0ul>, agiru::Text<0ul> >, bool, agiru::Enum<agiru::System::Integration::Word::WordTemplatesSaveFormat_Enum>)"); }
extern "C" void agiru_unlinked_546() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit5MergeENS_10DictionaryINS_4TextILm0EEES6_EEbNS_4EnumINS2_28WordTemplatesSaveFormat_EnumEEEbNS8_INS1_31DocSharingConflictBehavior_EnumEEE");
extern "C" void agiru_unlinked_546() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Merge(agiru::Dictionary<agiru::Text<0ul>, agiru::Text<0ul> >, bool, agiru::Enum<agiru::System::Integration::Word::WordTemplatesSaveFormat_Enum>, bool, agiru::Enum<agiru::System::Integration::DocSharingConflictBehavior_Enum>)"); }
extern "C" void agiru_unlinked_547() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit5MergeENS_7VariantEbNS_4EnumINS2_28WordTemplatesSaveFormat_EnumEEEb");
extern "C" void agiru_unlinked_547() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Merge(agiru::Variant, bool, agiru::Enum<agiru::System::Integration::Word::WordTemplatesSaveFormat_Enum>, bool)"); }
extern "C" void agiru_unlinked_548() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit5MergeENS_7VariantEbNS_4EnumINS2_28WordTemplatesSaveFormat_EnumEEEbNS5_INS1_31DocSharingConflictBehavior_EnumEEE");
extern "C" void agiru_unlinked_548() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Merge(agiru::Variant, bool, agiru::Enum<agiru::System::Integration::Word::WordTemplatesSaveFormat_Enum>, bool, agiru::Enum<agiru::System::Integration::DocSharingConflictBehavior_Enum>)"); }
extern "C" void agiru_unlinked_549() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit5MergeEbNS_4EnumINS2_28WordTemplatesSaveFormat_EnumEEE");
extern "C" void agiru_unlinked_549() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Merge(bool, agiru::Enum<agiru::System::Integration::Word::WordTemplatesSaveFormat_Enum>)"); }
extern "C" void agiru_unlinked_550() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit6CreateENS_4ListINS_4TextILm0EEEEE");
extern "C" void agiru_unlinked_550() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Create(agiru::List<agiru::Text<0ul> >)"); }
extern "C" void agiru_unlinked_551() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit6CreateEiNS_4ListIiEENS4_INS_4CodeILm5EEEEERNS2_23WordTemplateField_TableE");
extern "C" void agiru_unlinked_551() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Create(int, agiru::List<int>, agiru::List<agiru::Code<5ul> >, agiru::System::Integration::Word::WordTemplateField_Table&)"); }
extern "C" void agiru_unlinked_552() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit6CreateEiRNS2_23WordTemplateField_TableE");
extern "C" void agiru_unlinked_552() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Create(int, agiru::System::Integration::Word::WordTemplateField_Table&)"); }
extern "C" void agiru_unlinked_553() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit6CreateEv");
extern "C" void agiru_unlinked_553() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Create()"); }
extern "C" void agiru_unlinked_554() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit6UploadERNS2_18WordTemplate_TableERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_554() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::Upload(agiru::System::Integration::Word::WordTemplate_Table&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_555() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit8AddTableERNS2_32WordTemplatesRelatedBuffer_TableENS_4CodeILm30EEEiNS_4GuidENS6_ILm5EEERNS2_23WordTemplateField_TableE");
extern "C" void agiru_unlinked_555() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::AddTable(agiru::System::Integration::Word::WordTemplatesRelatedBuffer_Table&, agiru::Code<30ul>, int, agiru::Guid, agiru::Code<5ul>, agiru::System::Integration::Word::WordTemplateField_Table&)"); }
extern "C" void agiru_unlinked_556() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit8AddTableEi");
extern "C" void agiru_unlinked_556() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::AddTable(int)"); }
extern "C" void agiru_unlinked_557() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit8AddTableEv");
extern "C" void agiru_unlinked_557() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::AddTable()"); }
extern "C" void agiru_unlinked_558() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit8GetFieldENS_4TextILm0EEEiS5_");
extern "C" void agiru_unlinked_558() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GetField(agiru::Text<0ul>, int, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_559() asm("_ZN5agiru6System11Integration4Word25WordTemplateImpl_Codeunit8GetTableENS_4TextILm0EEERNS_8platform23AllObjWithCaption_TableES5_");
extern "C" void agiru_unlinked_559() { Unlinked("agiru::System::Integration::Word::WordTemplateImpl_Codeunit::GetTable(agiru::Text<0ul>, agiru::platform::AllObjWithCaption_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_560() asm("_ZN5agiru6System11Integration4Word32WordTemplateSelectionWizard_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_560() { Unlinked("agiru::System::Integration::Word::WordTemplateSelectionWizard_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_561() asm("_ZN5agiru6System11Integration4Word32WordTemplateSelectionWizard_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_561() { Unlinked("agiru::System::Integration::Word::WordTemplateSelectionWizard_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_562() asm("_ZN5agiru6System11Integration4Word32WordTemplateSelectionWizard_Page7SetDataENS_7VariantE");
extern "C" void agiru_unlinked_562() { Unlinked("agiru::System::Integration::Word::WordTemplateSelectionWizard_Page::SetData(agiru::Variant)"); }
extern "C" void agiru_unlinked_563() asm("_ZN5agiru6System11Integration7PowerBI24UploadPowerBIReport_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_563() { Unlinked("agiru::System::Integration::PowerBI::UploadPowerBIReport_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_564() asm("_ZN5agiru6System11Integration7PowerBI24UploadPowerBIReport_Page11OnClosePageEv");
extern "C" void agiru_unlinked_564() { Unlinked("agiru::System::Integration::PowerBI::UploadPowerBIReport_Page::OnClosePage()"); }
extern "C" void agiru_unlinked_565() asm("_ZN5agiru6System11Integration7PowerBI24UploadPowerBIReport_Page6OnInitEv");
extern "C" void agiru_unlinked_565() { Unlinked("agiru::System::Integration::PowerBI::UploadPowerBIReport_Page::OnInit()"); }
extern "C" void agiru_unlinked_566() asm("_ZN5agiru6System13TestLibraries7Mocking34MockGraphQueryTestLibrary_Codeunit19AddGraphUserToGroupENS_6dotnet8UserInfoENS_4TextILm0EEES7_");
extern "C" void agiru_unlinked_566() { Unlinked("agiru::System::TestLibraries::Mocking::MockGraphQueryTestLibrary_Codeunit::AddGraphUserToGroup(agiru::dotnet::UserInfo, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_567() asm("_ZN5agiru6System13TestLibraries7Mocking34MockGraphQueryTestLibrary_Codeunit19SetupMockGraphQueryEv");
extern "C" void agiru_unlinked_567() { Unlinked("agiru::System::TestLibraries::Mocking::MockGraphQueryTestLibrary_Codeunit::SetupMockGraphQuery()"); }
extern "C" void agiru_unlinked_568() asm("_ZN5agiru6System13TestLibraries7Mocking34MockGraphQueryTestLibrary_Codeunit21AddAndReturnGraphUserERNS_6dotnet8UserInfoENS_4TextILm0EEES8_S8_S8_");
extern "C" void agiru_unlinked_568() { Unlinked("agiru::System::TestLibraries::Mocking::MockGraphQueryTestLibrary_Codeunit::AddAndReturnGraphUser(agiru::dotnet::UserInfo&, agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_569() asm("_ZN5agiru6System13TestLibraries7Mocking34MockGraphQueryTestLibrary_Codeunit8AddGroupENS_4TextILm0EEES5_");
extern "C" void agiru_unlinked_569() { Unlinked("agiru::System::TestLibraries::Mocking::MockGraphQueryTestLibrary_Codeunit::AddGroup(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_570() asm("_ZN5agiru6System13Visualization14ChartList_Page13OnActionSetupEv");
extern "C" void agiru_unlinked_570() { Unlinked("agiru::System::Visualization::ChartList_Page::OnActionSetup()"); }
extern "C" void agiru_unlinked_571() asm("_ZN5agiru6System13Visualization14ChartList_Page14OnEnabledSetupEv");
extern "C" void agiru_unlinked_571() { Unlinked("agiru::System::Visualization::ChartList_Page::OnEnabledSetup()"); }
extern "C" void agiru_unlinked_572() asm("_ZN5agiru6System13Visualization14ChartList_Page16OnQueryClosePageENS_6ActionE");
extern "C" void agiru_unlinked_572() { Unlinked("agiru::System::Visualization::ChartList_Page::OnQueryClosePage(agiru::Action)"); }
extern "C" void agiru_unlinked_573() asm("_ZN5agiru6System13Visualization14ChartList_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_573() { Unlinked("agiru::System::Visualization::ChartList_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_574() asm("_ZN5agiru6System13Visualization24ChartManagement_Codeunit10AddinReadyERNS1_21ChartDefinition_TableERNS1_25BusinessChartBuffer_TableE");
extern "C" void agiru_unlinked_574() { Unlinked("agiru::System::Visualization::ChartManagement_Codeunit::AddinReady(agiru::System::Visualization::ChartDefinition_Table&, agiru::System::Visualization::BusinessChartBuffer_Table&)"); }
extern "C" void agiru_unlinked_575() asm("_ZN5agiru6System13Visualization24ChartManagement_Codeunit11SelectChartERNS1_25BusinessChartBuffer_TableERNS1_21ChartDefinition_TableE");
extern "C" void agiru_unlinked_575() { Unlinked("agiru::System::Visualization::ChartManagement_Codeunit::SelectChart(agiru::System::Visualization::BusinessChartBuffer_Table&, agiru::System::Visualization::ChartDefinition_Table&)"); }
extern "C" void agiru_unlinked_576() asm("_ZN5agiru6System13Visualization24ChartManagement_Codeunit15SetPeriodLengthENS1_21ChartDefinition_TableERNS1_25BusinessChartBuffer_TableENS_6OptionIvEEb");
extern "C" void agiru_unlinked_576() { Unlinked("agiru::System::Visualization::ChartManagement_Codeunit::SetPeriodLength(agiru::System::Visualization::ChartDefinition_Table, agiru::System::Visualization::BusinessChartBuffer_Table&, agiru::Option<void>, bool)"); }
extern "C" void agiru_unlinked_577() asm("_ZN5agiru6System13Visualization24ChartManagement_Codeunit16ChartDescriptionENS1_21ChartDefinition_TableE");
extern "C" void agiru_unlinked_577() { Unlinked("agiru::System::Visualization::ChartManagement_Codeunit::ChartDescription(agiru::System::Visualization::ChartDefinition_Table)"); }
extern "C" void agiru_unlinked_578() asm("_ZN5agiru6System13Visualization24ChartManagement_Codeunit16DataPointClickedERNS1_25BusinessChartBuffer_TableERNS1_21ChartDefinition_TableE");
extern "C" void agiru_unlinked_578() { Unlinked("agiru::System::Visualization::ChartManagement_Codeunit::DataPointClicked(agiru::System::Visualization::BusinessChartBuffer_Table&, agiru::System::Visualization::ChartDefinition_Table&)"); }
extern "C" void agiru_unlinked_579() asm("_ZN5agiru6System13Visualization24ChartManagement_Codeunit16UpdateStatusTextERNS1_21ChartDefinition_TableERNS1_25BusinessChartBuffer_TableERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_579() { Unlinked("agiru::System::Visualization::ChartManagement_Codeunit::UpdateStatusText(agiru::System::Visualization::ChartDefinition_Table&, agiru::System::Visualization::BusinessChartBuffer_Table&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_580() asm("_ZN5agiru6System13Visualization24ChartManagement_Codeunit28PopulateChartDefinitionTableEv");
extern "C" void agiru_unlinked_580() { Unlinked("agiru::System::Visualization::ChartManagement_Codeunit::PopulateChartDefinitionTable()"); }
extern "C" void agiru_unlinked_581() asm("_ZN5agiru6System13Visualization24ChartManagement_Codeunit30TopCustomerListUpdatedRecentlyERi");
extern "C" void agiru_unlinked_581() { Unlinked("agiru::System::Visualization::ChartManagement_Codeunit::TopCustomerListUpdatedRecently(int&)"); }
extern "C" void agiru_unlinked_582() asm("_ZN5agiru6System13Visualization24ChartManagement_Codeunit34ScheduleTopCustomerListRefreshTaskEv");
extern "C" void agiru_unlinked_582() { Unlinked("agiru::System::Visualization::ChartManagement_Codeunit::ScheduleTopCustomerListRefreshTask()"); }
extern "C" void agiru_unlinked_583() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit11RetrieveXMLERNS_6absent5ChartERNS1_23GenericChartSetup_TableERNS1_23GenericChartYAxis_TableERNS1_32GenericChartCaptionsBuffer_TableERNS1_28GenericChartMemoBuffer_TableERNS1_24GenericChartFilter_TableE");
extern "C" void agiru_unlinked_583() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::RetrieveXML(agiru::absent::Chart&, agiru::System::Visualization::GenericChartSetup_Table&, agiru::System::Visualization::GenericChartYAxis_Table&, agiru::System::Visualization::GenericChartCaptionsBuffer_Table&, agiru::System::Visualization::GenericChartMemoBuffer_Table&, agiru::System::Visualization::GenericChartFilter_Table&)"); }
extern "C" void agiru_unlinked_584() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit11SaveChangesERNS_6absent5ChartENS1_23GenericChartSetup_TableERNS1_23GenericChartYAxis_TableERNS1_24GenericChartFilter_TableERNS1_32GenericChartCaptionsBuffer_TableERNS1_28GenericChartMemoBuffer_TableE");
extern "C" void agiru_unlinked_584() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::SaveChanges(agiru::absent::Chart&, agiru::System::Visualization::GenericChartSetup_Table, agiru::System::Visualization::GenericChartYAxis_Table&, agiru::System::Visualization::GenericChartFilter_Table&, agiru::System::Visualization::GenericChartCaptionsBuffer_Table&, agiru::System::Visualization::GenericChartMemoBuffer_Table&)"); }
extern "C" void agiru_unlinked_585() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit14LookUpObjectIdENS_6OptionINS_7options21OptionBlankTableQueryEEERiRNS_4TextILm0EEE");
extern "C" void agiru_unlinked_585() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::LookUpObjectId(agiru::Option<agiru::options::OptionBlankTableQuery>, int&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_586() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit14XAxisTitleCodeEv");
extern "C" void agiru_unlinked_586() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::XAxisTitleCode()"); }
extern "C" void agiru_unlinked_587() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit14YAxisTitleCodeEv");
extern "C" void agiru_unlinked_587() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::YAxisTitleCode()"); }
extern "C" void agiru_unlinked_588() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit15BuildFilterTextERNS_4TextILm0EEENS3_ILm100EEE");
extern "C" void agiru_unlinked_588() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::BuildFilterText(agiru::Text<0ul>&, agiru::Text<100ul>)"); }
extern "C" void agiru_unlinked_589() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit15DescriptionCodeEv");
extern "C" void agiru_unlinked_589() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::DescriptionCode()"); }
extern "C" void agiru_unlinked_590() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit15FillChartHelperERNS_6dotnet20BusinessChartBuilderENS1_23GenericChartSetup_TableERNS1_23GenericChartYAxis_TableERNS1_24GenericChartFilter_TableERNS1_32GenericChartCaptionsBuffer_TableERNS1_28GenericChartMemoBuffer_TableE");
extern "C" void agiru_unlinked_590() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::FillChartHelper(agiru::dotnet::BusinessChartBuilder&, agiru::System::Visualization::GenericChartSetup_Table, agiru::System::Visualization::GenericChartYAxis_Table&, agiru::System::Visualization::GenericChartFilter_Table&, agiru::System::Visualization::GenericChartCaptionsBuffer_Table&, agiru::System::Visualization::GenericChartMemoBuffer_Table&)"); }
extern "C" void agiru_unlinked_591() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit15GetUserLanguageEv");
extern "C" void agiru_unlinked_591() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::GetUserLanguage()"); }
extern "C" void agiru_unlinked_592() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit16MemoMLAssistEditERNS1_28GenericChartMemoBuffer_TableENS_4CodeILm10EEE");
extern "C" void agiru_unlinked_592() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::MemoMLAssistEdit(agiru::System::Visualization::GenericChartMemoBuffer_Table&, agiru::Code<10ul>)"); }
extern "C" void agiru_unlinked_593() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit16TextMLAssistEditERNS1_32GenericChartCaptionsBuffer_TableENS_4CodeILm10EEE");
extern "C" void agiru_unlinked_593() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::TextMLAssistEdit(agiru::System::Visualization::GenericChartCaptionsBuffer_Table&, agiru::Code<10ul>)"); }
extern "C" void agiru_unlinked_594() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit16ValidateObjectIDENS_6OptionINS_7options21OptionBlankTableQueryEEERiRNS_4TextILm0EEE");
extern "C" void agiru_unlinked_594() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::ValidateObjectID(agiru::Option<agiru::options::OptionBlankTableQuery>, int&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_595() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit16XAxisCaptionCodeEv");
extern "C" void agiru_unlinked_595() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::XAxisCaptionCode()"); }
extern "C" void agiru_unlinked_596() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit16ZAxisCaptionCodeEv");
extern "C" void agiru_unlinked_596() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::ZAxisCaptionCode()"); }
extern "C" void agiru_unlinked_597() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit17CheckSourceTypeIDENS1_23GenericChartSetup_TableEb");
extern "C" void agiru_unlinked_597() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::CheckSourceTypeID(agiru::System::Visualization::GenericChartSetup_Table, bool)"); }
extern "C" void agiru_unlinked_598() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit18FinalizeFilterTextERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_598() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::FinalizeFilterText(agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_599() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit19RequiredMeasureCodeEv");
extern "C" void agiru_unlinked_599() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::RequiredMeasureCode()"); }
extern "C" void agiru_unlinked_600() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit19RetrieveFieldColumnENS1_23GenericChartSetup_TableERiRNS_4TextILm0EEES7_ib");
extern "C" void agiru_unlinked_600() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::RetrieveFieldColumn(agiru::System::Visualization::GenericChartSetup_Table, int&, agiru::Text<0ul>&, agiru::Text<0ul>&, int, bool)"); }
extern "C" void agiru_unlinked_601() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit19ValidateFieldColumnENS1_23GenericChartSetup_TableERiNS_4TextILm80EEERNS5_ILm0EEEibRNS_6OptionINS_7options27OptionNoneCountSumMinMaxAvgEEE");
extern "C" void agiru_unlinked_601() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::ValidateFieldColumn(agiru::System::Visualization::GenericChartSetup_Table, int&, agiru::Text<80ul>, agiru::Text<0ul>&, int, bool, agiru::Option<agiru::options::OptionNoneCountSumMinMaxAvg>&)"); }
extern "C" void agiru_unlinked_602() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit20OptionalMeasure1CodeEv");
extern "C" void agiru_unlinked_602() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::OptionalMeasure1Code()"); }
extern "C" void agiru_unlinked_603() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit20OptionalMeasure2CodeEv");
extern "C" void agiru_unlinked_603() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::OptionalMeasure2Code()"); }
extern "C" void agiru_unlinked_604() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit20OptionalMeasure3CodeEv");
extern "C" void agiru_unlinked_604() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::OptionalMeasure3Code()"); }
extern "C" void agiru_unlinked_605() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit20OptionalMeasure4CodeEv");
extern "C" void agiru_unlinked_605() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::OptionalMeasure4Code()"); }
extern "C" void agiru_unlinked_606() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit20OptionalMeasure5CodeEv");
extern "C" void agiru_unlinked_606() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::OptionalMeasure5Code()"); }
extern "C" void agiru_unlinked_607() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit23GetQueryCountColumnNameERNS1_23GenericChartSetup_TableE");
extern "C" void agiru_unlinked_607() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::GetQueryCountColumnName(agiru::System::Visualization::GenericChartSetup_Table&)"); }
extern "C" void agiru_unlinked_608() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit29RetrieveFieldColumnIDFromNameENS_6OptionINS_7options21OptionBlankTableQueryEEEiRiNS_4TextILm50EEE");
extern "C" void agiru_unlinked_608() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::RetrieveFieldColumnIDFromName(agiru::Option<agiru::options::OptionBlankTableQuery>, int, int&, agiru::Text<50ul>)"); }
extern "C" void agiru_unlinked_609() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit34CheckDataTypeAggregationComplianceENS1_23GenericChartSetup_TableENS_4TextILm50EEENS_6OptionINS_7options27OptionNoneCountSumMinMaxAvgEEE");
extern "C" void agiru_unlinked_609() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::CheckDataTypeAggregationCompliance(agiru::System::Visualization::GenericChartSetup_Table, agiru::Text<50ul>, agiru::Option<agiru::options::OptionNoneCountSumMinMaxAvg>)"); }
extern "C" void agiru_unlinked_610() asm("_ZN5agiru6System13Visualization24GenericChartMgt_Codeunit9CopyChartERNS_6absent5ChartENS_4CodeILm20EEENS_4TextILm50EEE");
extern "C" void agiru_unlinked_610() { Unlinked("agiru::System::Visualization::GenericChartMgt_Codeunit::CopyChart(agiru::absent::Chart&, agiru::Code<20ul>, agiru::Text<50ul>)"); }
extern "C" void agiru_unlinked_611() asm("_ZN5agiru6System18DataAdministration21TableInformation_Page6OnInitEv");
extern "C" void agiru_unlinked_611() { Unlinked("agiru::System::DataAdministration::TableInformation_Page::OnInit()"); }
extern "C" void agiru_unlinked_612() asm("_ZN5agiru6System18DataAdministration25TableInformationCard_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_612() { Unlinked("agiru::System::DataAdministration::TableInformationCard_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_613() asm("_ZN5agiru6System2IO19ConfigPackages_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_613() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_614() asm("_ZN5agiru6System2IO19ConfigPackages_Page17OnActionGetTablesEv");
extern "C" void agiru_unlinked_614() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnActionGetTables()"); }
extern "C" void agiru_unlinked_615() asm("_ZN5agiru6System2IO19ConfigPackages_Page19OnActionCopyPackageEv");
extern "C" void agiru_unlinked_615() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnActionCopyPackage()"); }
extern "C" void agiru_unlinked_616() asm("_ZN5agiru6System2IO19ConfigPackages_Page20OnActionApplyPackageEv");
extern "C" void agiru_unlinked_616() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnActionApplyPackage()"); }
extern "C" void agiru_unlinked_617() asm("_ZN5agiru6System2IO19ConfigPackages_Page21OnActionExportPackageEv");
extern "C" void agiru_unlinked_617() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnActionExportPackage()"); }
extern "C" void agiru_unlinked_618() asm("_ZN5agiru6System2IO19ConfigPackages_Page21OnActionExportToExcelEv");
extern "C" void agiru_unlinked_618() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnActionExportToExcel()"); }
extern "C" void agiru_unlinked_619() asm("_ZN5agiru6System2IO19ConfigPackages_Page21OnActionImportPackageEv");
extern "C" void agiru_unlinked_619() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnActionImportPackage()"); }
extern "C" void agiru_unlinked_620() asm("_ZN5agiru6System2IO19ConfigPackages_Page23OnActionImportFromExcelEv");
extern "C" void agiru_unlinked_620() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnActionImportFromExcel()"); }
extern "C" void agiru_unlinked_621() asm("_ZN5agiru6System2IO19ConfigPackages_Page23OnActionValidatePackageEv");
extern "C" void agiru_unlinked_621() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnActionValidatePackage()"); }
extern "C" void agiru_unlinked_622() asm("_ZN5agiru6System2IO19ConfigPackages_Page27OnActionExportToTranslationEv");
extern "C" void agiru_unlinked_622() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnActionExportToTranslation()"); }
extern "C" void agiru_unlinked_623() asm("_ZN5agiru6System2IO19ConfigPackages_Page31OnActionImportPredefinedPackageEv");
extern "C" void agiru_unlinked_623() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnActionImportPredefinedPackage()"); }
extern "C" void agiru_unlinked_624() asm("_ZN5agiru6System2IO19ConfigPackages_Page32OnVisibleImportPredefinedPackageEv");
extern "C" void agiru_unlinked_624() { Unlinked("agiru::System::IO::ConfigPackages_Page::OnVisibleImportPredefinedPackage()"); }
extern "C" void agiru_unlinked_625() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit10InitializeEv");
extern "C" void agiru_unlinked_625() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::Initialize()"); }
extern "C" void agiru_unlinked_626() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit13AddAttachmentENS_4TextILm0EEENS_4EnumINS1_30PDFAttachDataRelationship_EnumEEES4_NS_8InStreamES4_b");
extern "C" void agiru_unlinked_626() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::AddAttachment(agiru::Text<0ul>, agiru::Enum<agiru::System::IO::PDFAttachDataRelationship_Enum>, agiru::Text<0ul>, agiru::InStream, agiru::Text<0ul>, bool)"); }
extern "C" void agiru_unlinked_627() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit13AddAttachmentENS_4TextILm0EEENS_4EnumINS1_30PDFAttachDataRelationship_EnumEEES4_S4_S4_b");
extern "C" void agiru_unlinked_627() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::AddAttachment(agiru::Text<0ul>, agiru::Enum<agiru::System::IO::PDFAttachDataRelationship_Enum>, agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>, bool)"); }
extern "C" void agiru_unlinked_628() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit13GetZipArchiveENS_8InStreamE");
extern "C" void agiru_unlinked_628() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::GetZipArchive(agiru::InStream)"); }
extern "C" void agiru_unlinked_629() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit14ConvertToImageERNS_8InStreamENS_4EnumINS0_9Utilities16ImageFormat_EnumEEEi");
extern "C" void agiru_unlinked_629() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::ConvertToImage(agiru::InStream&, agiru::Enum<agiru::System::Utilities::ImageFormat_Enum>, int)"); }
extern "C" void agiru_unlinked_630() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit15AddFileToAppendENS_4TextILm0EEE");
extern "C" void agiru_unlinked_630() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::AddFileToAppend(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_631() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit15AttachmentCountEv");
extern "C" void agiru_unlinked_631() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::AttachmentCount()"); }
extern "C" void agiru_unlinked_632() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit15GetPdfPageCountENS_8InStreamE");
extern "C" void agiru_unlinked_632() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::GetPdfPageCount(agiru::InStream)"); }
extern "C" void agiru_unlinked_633() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit15ProtectDocumentENS_10SecretTextES3_");
extern "C" void agiru_unlinked_633() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::ProtectDocument(agiru::SecretText, agiru::SecretText)"); }
extern "C" void agiru_unlinked_634() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit15ProtectDocumentENS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_634() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::ProtectDocument(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_635() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit16GetPdfPropertiesENS_8InStreamE");
extern "C" void agiru_unlinked_635() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::GetPdfProperties(agiru::InStream)"); }
extern "C" void agiru_unlinked_636() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit17AddStreamToAppendENS_8InStreamE");
extern "C" void agiru_unlinked_636() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::AddStreamToAppend(agiru::InStream)"); }
extern "C" void agiru_unlinked_637() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit18GetAttachmentNamesENS_8InStreamE");
extern "C" void agiru_unlinked_637() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::GetAttachmentNames(agiru::InStream)"); }
extern "C" void agiru_unlinked_638() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit21AppendedDocumentCountEv");
extern "C" void agiru_unlinked_638() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::AppendedDocumentCount()"); }
extern "C" void agiru_unlinked_639() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit27GetDocumentAttachmentStreamENS_8InStreamERNS0_9Utilities17TempBlob_CodeunitE");
extern "C" void agiru_unlinked_639() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::GetDocumentAttachmentStream(agiru::InStream, agiru::System::Utilities::TempBlob_Codeunit&)"); }
extern "C" void agiru_unlinked_640() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit4LoadENS_8InStreamE");
extern "C" void agiru_unlinked_640() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::Load(agiru::InStream)"); }
extern "C" void agiru_unlinked_641() asm("_ZN5agiru6System2IO24PDFDocumentImpl_Codeunit6ToJsonENS_10JsonObjectE");
extern "C" void agiru_unlinked_641() { Unlinked("agiru::System::IO::PDFDocumentImpl_Codeunit::ToJson(agiru::JsonObject)"); }
extern "C" void agiru_unlinked_642() asm("_ZN5agiru6System2IO24XMLBufferWriter_Codeunit13InsertElementERNS1_15XMLBuffer_TableES3_iiNS_4TextILm0EEES6_");
extern "C" void agiru_unlinked_642() { Unlinked("agiru::System::IO::XMLBufferWriter_Codeunit::InsertElement(agiru::System::IO::XMLBuffer_Table&, agiru::System::IO::XMLBuffer_Table, int, int, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_643() asm("_ZN5agiru6System2IO24XMLBufferWriter_Codeunit15InsertAttributeERNS1_15XMLBuffer_TableES3_iiNS_4TextILm0EEES6_");
extern "C" void agiru_unlinked_643() { Unlinked("agiru::System::IO::XMLBufferWriter_Codeunit::InsertAttribute(agiru::System::IO::XMLBuffer_Table&, agiru::System::IO::XMLBuffer_Table, int, int, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_644() asm("_ZN5agiru6System2IO24XMLBufferWriter_Codeunit23InitializeXMLBufferFromERNS1_15XMLBuffer_TableENS_7VariantE");
extern "C" void agiru_unlinked_644() { Unlinked("agiru::System::IO::XMLBufferWriter_Codeunit::InitializeXMLBufferFrom(agiru::System::IO::XMLBuffer_Table&, agiru::Variant)"); }
extern "C" void agiru_unlinked_645() asm("_ZN5agiru6System2IO24XMLBufferWriter_Codeunit25GenerateStructureFromPathERNS1_15XMLBuffer_TableENS_4TextILm0EEE");
extern "C" void agiru_unlinked_645() { Unlinked("agiru::System::IO::XMLBufferWriter_Codeunit::GenerateStructureFromPath(agiru::System::IO::XMLBuffer_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_646() asm("_ZN5agiru6System2IO24XMLBufferWriter_Codeunit27InitializeXMLBufferFromTextERNS1_15XMLBuffer_TableENS_4TextILm0EEE");
extern "C" void agiru_unlinked_646() { Unlinked("agiru::System::IO::XMLBufferWriter_Codeunit::InitializeXMLBufferFromText(agiru::System::IO::XMLBuffer_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_647() asm("_ZN5agiru6System2IO24XMLBufferWriter_Codeunit27InsertProcessingInstructionERNS1_15XMLBuffer_TableES3_iiNS_4TextILm0EEES6_");
extern "C" void agiru_unlinked_647() { Unlinked("agiru::System::IO::XMLBufferWriter_Codeunit::InsertProcessingInstruction(agiru::System::IO::XMLBuffer_Table&, agiru::System::IO::XMLBuffer_Table, int, int, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_648() asm("_ZN5agiru6System2IO24XMLBufferWriter_Codeunit28InsertAttributeWithNamespaceERNS1_15XMLBuffer_TableES3_iiNS_4TextILm0EEES6_");
extern "C" void agiru_unlinked_648() { Unlinked("agiru::System::IO::XMLBufferWriter_Codeunit::InsertAttributeWithNamespace(agiru::System::IO::XMLBuffer_Table&, agiru::System::IO::XMLBuffer_Table, int, int, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_649() asm("_ZN5agiru6System2IO24XMLBufferWriter_Codeunit29InitializeXMLBufferFromStreamERNS1_15XMLBuffer_TableENS_8InStreamE");
extern "C" void agiru_unlinked_649() { Unlinked("agiru::System::IO::XMLBufferWriter_Codeunit::InitializeXMLBufferFromStream(agiru::System::IO::XMLBuffer_Table&, agiru::InStream)"); }
extern "C" void agiru_unlinked_650() asm("_ZN5agiru6System2IO25GetJsonStructure_Codeunit17GenerateStructureENS_4TextILm0EEERNS1_15XMLBuffer_TableE");
extern "C" void agiru_unlinked_650() { Unlinked("agiru::System::IO::GetJsonStructure_Codeunit::GenerateStructure(agiru::Text<0ul>, agiru::System::IO::XMLBuffer_Table&)"); }
extern "C" void agiru_unlinked_651() asm("_ZN5agiru6System2IO25GetJsonStructure_Codeunit26JsonToXMLCreateDefaultRootENS_8InStreamERNS_9OutStreamE");
extern "C" void agiru_unlinked_651() { Unlinked("agiru::System::IO::GetJsonStructure_Codeunit::JsonToXMLCreateDefaultRoot(agiru::InStream, agiru::OutStream&)"); }
extern "C" void agiru_unlinked_652() asm("_ZN5agiru6System2IO25GetJsonStructure_Codeunit9JsonToXMLENS_8InStreamERNS_9OutStreamE");
extern "C" void agiru_unlinked_652() { Unlinked("agiru::System::IO::GetJsonStructure_Codeunit::JsonToXML(agiru::InStream, agiru::OutStream&)"); }
extern "C" void agiru_unlinked_653() asm("_ZN5agiru6System2IO28ConfigExcelExchange_Codeunit11ExportExcelERNS_4TextILm0EEERNS1_24ConfigPackageTable_TableEbb");
extern "C" void agiru_unlinked_653() { Unlinked("agiru::System::IO::ConfigExcelExchange_Codeunit::ExportExcel(agiru::Text<0ul>&, agiru::System::IO::ConfigPackageTable_Table&, bool, bool)"); }
extern "C" void agiru_unlinked_654() asm("_ZN5agiru6System2IO28ConfigExcelExchange_Codeunit13SetHideDialogEb");
extern "C" void agiru_unlinked_654() { Unlinked("agiru::System::IO::ConfigExcelExchange_Codeunit::SetHideDialog(bool)"); }
extern "C" void agiru_unlinked_655() asm("_ZN5agiru6System2IO28ConfigExcelExchange_Codeunit15SetFileOnServerEb");
extern "C" void agiru_unlinked_655() { Unlinked("agiru::System::IO::ConfigExcelExchange_Codeunit::SetFileOnServer(bool)"); }
extern "C" void agiru_unlinked_656() asm("_ZN5agiru6System2IO28ConfigExcelExchange_Codeunit17SetSelectedTablesERNS1_24ConfigPackageTable_TableE");
extern "C" void agiru_unlinked_656() { Unlinked("agiru::System::IO::ConfigExcelExchange_Codeunit::SetSelectedTables(agiru::System::IO::ConfigPackageTable_Table&)"); }
extern "C" void agiru_unlinked_657() asm("_ZN5agiru6System2IO28ConfigExcelExchange_Codeunit21ExportExcelFromConfigERNS1_16ConfigLine_TableE");
extern "C" void agiru_unlinked_657() { Unlinked("agiru::System::IO::ConfigExcelExchange_Codeunit::ExportExcelFromConfig(agiru::System::IO::ConfigLine_Table&)"); }
extern "C" void agiru_unlinked_658() asm("_ZN5agiru6System2IO28ConfigExcelExchange_Codeunit21ExportExcelFromTablesERNS1_24ConfigPackageTable_TableE");
extern "C" void agiru_unlinked_658() { Unlinked("agiru::System::IO::ConfigExcelExchange_Codeunit::ExportExcelFromTables(agiru::System::IO::ConfigPackageTable_Table&)"); }
extern "C" void agiru_unlinked_659() asm("_ZN5agiru6System2IO28ConfigExcelExchange_Codeunit21ImportExcelFromConfigENS1_16ConfigLine_TableE");
extern "C" void agiru_unlinked_659() { Unlinked("agiru::System::IO::ConfigExcelExchange_Codeunit::ImportExcelFromConfig(agiru::System::IO::ConfigLine_Table)"); }
extern "C" void agiru_unlinked_660() asm("_ZN5agiru6System2IO28ConfigExcelExchange_Codeunit30ImportExcelFromSelectedPackageENS_4CodeILm20EEE");
extern "C" void agiru_unlinked_660() { Unlinked("agiru::System::IO::ConfigExcelExchange_Codeunit::ImportExcelFromSelectedPackage(agiru::Code<20ul>)"); }
extern "C" void agiru_unlinked_661() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit11RemoveEntryENS_4TextILm0EEE");
extern "C" void agiru_unlinked_661() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::RemoveEntry(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_662() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit12ExtractEntryENS_4TextILm0EEENS_9OutStreamERi");
extern "C" void agiru_unlinked_662() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::ExtractEntry(agiru::Text<0ul>, agiru::OutStream, int&)"); }
extern "C" void agiru_unlinked_663() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit12ExtractEntryENS_4TextILm0EEERNS0_9Utilities17TempBlob_CodeunitE");
extern "C" void agiru_unlinked_663() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::ExtractEntry(agiru::Text<0ul>, agiru::System::Utilities::TempBlob_Codeunit&)"); }
extern "C" void agiru_unlinked_664() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit12GZipCompressENS_8InStreamENS_9OutStreamE");
extern "C" void agiru_unlinked_664() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::GZipCompress(agiru::InStream, agiru::OutStream)"); }
extern "C" void agiru_unlinked_665() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit12GetEntryListERNS_4ListINS_4TextILm0EEEEE");
extern "C" void agiru_unlinked_665() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::GetEntryList(agiru::List<agiru::Text<0ul> >&)"); }
extern "C" void agiru_unlinked_666() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit14GZipDecompressENS_8InStreamENS_9OutStreamE");
extern "C" void agiru_unlinked_666() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::GZipDecompress(agiru::InStream, agiru::OutStream)"); }
extern "C" void agiru_unlinked_667() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit14OpenZipArchiveENS0_9Utilities17TempBlob_CodeunitEb");
extern "C" void agiru_unlinked_667() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::OpenZipArchive(agiru::System::Utilities::TempBlob_Codeunit, bool)"); }
extern "C" void agiru_unlinked_668() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit14OpenZipArchiveENS_8InStreamEb");
extern "C" void agiru_unlinked_668() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::OpenZipArchive(agiru::InStream, bool)"); }
extern "C" void agiru_unlinked_669() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit14OpenZipArchiveENS_8InStreamEbi");
extern "C" void agiru_unlinked_669() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::OpenZipArchive(agiru::InStream, bool, int)"); }
extern "C" void agiru_unlinked_670() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit14SaveZipArchiveENS_9OutStreamE");
extern "C" void agiru_unlinked_670() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::SaveZipArchive(agiru::OutStream)"); }
extern "C" void agiru_unlinked_671() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit14SaveZipArchiveERNS0_9Utilities17TempBlob_CodeunitE");
extern "C" void agiru_unlinked_671() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::SaveZipArchive(agiru::System::Utilities::TempBlob_Codeunit&)"); }
extern "C" void agiru_unlinked_672() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit15CloseZipArchiveEv");
extern "C" void agiru_unlinked_672() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::CloseZipArchive()"); }
extern "C" void agiru_unlinked_673() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit16CreateZipArchiveEv");
extern "C" void agiru_unlinked_673() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::CreateZipArchive()"); }
extern "C" void agiru_unlinked_674() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit5IsZipENS_8InStreamE");
extern "C" void agiru_unlinked_674() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::IsZip(agiru::InStream)"); }
extern "C" void agiru_unlinked_675() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit6IsGZipENS_8InStreamE");
extern "C" void agiru_unlinked_675() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::IsGZip(agiru::InStream)"); }
extern "C" void agiru_unlinked_676() asm("_ZN5agiru6System2IO28DataCompressionImpl_Codeunit8AddEntryENS_8InStreamENS_4TextILm0EEE");
extern "C" void agiru_unlinked_676() { Unlinked("agiru::System::IO::DataCompressionImpl_Codeunit::AddEntry(agiru::InStream, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_677() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit11ApplyAnswerENS1_24ConfigQuestionArea_TableE");
extern "C" void agiru_unlinked_677() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::ApplyAnswer(agiru::System::IO::ConfigQuestionArea_Table)"); }
extern "C" void agiru_unlinked_678() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit12ApplyAnswersENS1_25ConfigQuestionnaire_TableE");
extern "C" void agiru_unlinked_678() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::ApplyAnswers(agiru::System::IO::ConfigQuestionnaire_Table)"); }
extern "C" void agiru_unlinked_679() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit14GetElementNameENS_4TextILm0EEE");
extern "C" void agiru_unlinked_679() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::GetElementName(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_680() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit15UpdateQuestionsENS1_24ConfigQuestionArea_TableE");
extern "C" void agiru_unlinked_680() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::UpdateQuestions(agiru::System::IO::ConfigQuestionArea_Table)"); }
extern "C" void agiru_unlinked_681() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit17BuildAnswerOptionEii");
extern "C" void agiru_unlinked_681() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::BuildAnswerOption(int, int)"); }
extern "C" void agiru_unlinked_682() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit17SetCalledFromCodeEv");
extern "C" void agiru_unlinked_682() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::SetCalledFromCode()"); }
extern "C" void agiru_unlinked_683() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit19UpdateQuestionnaireENS1_25ConfigQuestionnaire_TableE");
extern "C" void agiru_unlinked_683() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::UpdateQuestionnaire(agiru::System::IO::ConfigQuestionnaire_Table)"); }
extern "C" void agiru_unlinked_684() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit24ExportQuestionnaireAsXMLENS_4TextILm0EEERNS1_25ConfigQuestionnaire_TableE");
extern "C" void agiru_unlinked_684() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::ExportQuestionnaireAsXML(agiru::Text<0ul>, agiru::System::IO::ConfigQuestionnaire_Table&)"); }
extern "C" void agiru_unlinked_685() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit24ImportQuestionnaireAsXMLENS_4TextILm0EEE");
extern "C" void agiru_unlinked_685() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::ImportQuestionnaireAsXML(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_686() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit26ExportQuestionnaireToExcelENS_4TextILm0EEERNS1_25ConfigQuestionnaire_TableE");
extern "C" void agiru_unlinked_686() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::ExportQuestionnaireToExcel(agiru::Text<0ul>, agiru::System::IO::ConfigQuestionnaire_Table&)"); }
extern "C" void agiru_unlinked_687() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit26ModifyConfigQuestionAnswerERNS1_20ConfigQuestion_TableENS_8platform5FieldE");
extern "C" void agiru_unlinked_687() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::ModifyConfigQuestionAnswer(agiru::System::IO::ConfigQuestion_Table&, agiru::platform::Field)"); }
extern "C" void agiru_unlinked_688() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit30ImportQuestionnaireXMLDocumentENS_6dotnet11XmlDocumentE");
extern "C" void agiru_unlinked_688() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::ImportQuestionnaireXMLDocument(agiru::dotnet::XmlDocument)"); }
extern "C" void agiru_unlinked_689() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit32GenerateQuestionnaireXMLDocumentENS_6dotnet11XmlDocumentERNS1_25ConfigQuestionnaire_TableE");
extern "C" void agiru_unlinked_689() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::GenerateQuestionnaireXMLDocument(agiru::dotnet::XmlDocument, agiru::System::IO::ConfigQuestionnaire_Table&)"); }
extern "C" void agiru_unlinked_690() asm("_ZN5agiru6System2IO32QuestionnaireManagement_Codeunit34ImportQuestionnaireAsXMLFromClientEv");
extern "C" void agiru_unlinked_690() { Unlinked("agiru::System::IO::QuestionnaireManagement_Codeunit::ImportQuestionnaireAsXMLFromClient()"); }
extern "C" void agiru_unlinked_691() asm("_ZN5agiru6System3Xml18XSDParser_Codeunit10LoadSchemaERNS1_15XMLSchema_TableE");
extern "C" void agiru_unlinked_691() { Unlinked("agiru::System::Xml::XSDParser_Codeunit::LoadSchema(agiru::System::Xml::XMLSchema_Table&)"); }
extern "C" void agiru_unlinked_692() asm("_ZN5agiru6System3Xml18XSDParser_Codeunit11DeselectAllERNS1_22XMLSchemaElement_TableE");
extern "C" void agiru_unlinked_692() { Unlinked("agiru::System::Xml::XSDParser_Codeunit::DeselectAll(agiru::System::Xml::XMLSchemaElement_Table&)"); }
extern "C" void agiru_unlinked_693() asm("_ZN5agiru6System3Xml18XSDParser_Codeunit15HideNotSelectedERNS1_22XMLSchemaElement_TableE");
extern "C" void agiru_unlinked_693() { Unlinked("agiru::System::Xml::XSDParser_Codeunit::HideNotSelected(agiru::System::Xml::XMLSchemaElement_Table&)"); }
extern "C" void agiru_unlinked_694() asm("_ZN5agiru6System3Xml18XSDParser_Codeunit15SelectMandatoryENS1_22XMLSchemaElement_TableE");
extern "C" void agiru_unlinked_694() { Unlinked("agiru::System::Xml::XSDParser_Codeunit::SelectMandatory(agiru::System::Xml::XMLSchemaElement_Table)"); }
extern "C" void agiru_unlinked_695() asm("_ZN5agiru6System3Xml18XSDParser_Codeunit16HideNotMandatoryERNS1_22XMLSchemaElement_TableE");
extern "C" void agiru_unlinked_695() { Unlinked("agiru::System::Xml::XSDParser_Codeunit::HideNotMandatory(agiru::System::Xml::XMLSchemaElement_Table&)"); }
extern "C" void agiru_unlinked_696() asm("_ZN5agiru6System3Xml18XSDParser_Codeunit21ExtendSelectedElementERNS1_22XMLSchemaElement_TableE");
extern "C" void agiru_unlinked_696() { Unlinked("agiru::System::Xml::XSDParser_Codeunit::ExtendSelectedElement(agiru::System::Xml::XMLSchemaElement_Table&)"); }
extern "C" void agiru_unlinked_697() asm("_ZN5agiru6System3Xml18XSDParser_Codeunit24CreateDataExchDefForCAMTERNS1_22XMLSchemaElement_TableE");
extern "C" void agiru_unlinked_697() { Unlinked("agiru::System::Xml::XSDParser_Codeunit::CreateDataExchDefForCAMT(agiru::System::Xml::XMLSchemaElement_Table&)"); }
extern "C" void agiru_unlinked_698() asm("_ZN5agiru6System3Xml18XSDParser_Codeunit7ShowAllERNS1_22XMLSchemaElement_TableE");
extern "C" void agiru_unlinked_698() { Unlinked("agiru::System::Xml::XSDParser_Codeunit::ShowAll(agiru::System::Xml::XMLSchemaElement_Table&)"); }
extern "C" void agiru_unlinked_699() asm("_ZN5agiru6System3Xml26XmlValidationImpl_Codeunit19AddValidationSchemaENS_11XmlDocumentENS_4TextILm0EEE");
extern "C" void agiru_unlinked_699() { Unlinked("agiru::System::Xml::XmlValidationImpl_Codeunit::AddValidationSchema(agiru::XmlDocument, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_700() asm("_ZN5agiru6System3Xml26XmlValidationImpl_Codeunit19AddValidationSchemaENS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_700() { Unlinked("agiru::System::Xml::XmlValidationImpl_Codeunit::AddValidationSchema(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_701() asm("_ZN5agiru6System3Xml26XmlValidationImpl_Codeunit19AddValidationSchemaENS_8InStreamENS_4TextILm0EEE");
extern "C" void agiru_unlinked_701() { Unlinked("agiru::System::Xml::XmlValidationImpl_Codeunit::AddValidationSchema(agiru::InStream, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_702() asm("_ZN5agiru6System3Xml26XmlValidationImpl_Codeunit20SetValidatedDocumentENS_11XmlDocumentE");
extern "C" void agiru_unlinked_702() { Unlinked("agiru::System::Xml::XmlValidationImpl_Codeunit::SetValidatedDocument(agiru::XmlDocument)"); }
extern "C" void agiru_unlinked_703() asm("_ZN5agiru6System3Xml26XmlValidationImpl_Codeunit20SetValidatedDocumentENS_4TextILm0EEE");
extern "C" void agiru_unlinked_703() { Unlinked("agiru::System::Xml::XmlValidationImpl_Codeunit::SetValidatedDocument(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_704() asm("_ZN5agiru6System3Xml26XmlValidationImpl_Codeunit20SetValidatedDocumentENS_8InStreamE");
extern "C" void agiru_unlinked_704() { Unlinked("agiru::System::Xml::XmlValidationImpl_Codeunit::SetValidatedDocument(agiru::InStream)"); }
extern "C" void agiru_unlinked_705() asm("_ZN5agiru6System3Xml26XmlValidationImpl_Codeunit21ValidateAgainstSchemaENS_11XmlDocumentES3_NS_4TextILm0EEE");
extern "C" void agiru_unlinked_705() { Unlinked("agiru::System::Xml::XmlValidationImpl_Codeunit::ValidateAgainstSchema(agiru::XmlDocument, agiru::XmlDocument, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_706() asm("_ZN5agiru6System3Xml26XmlValidationImpl_Codeunit21ValidateAgainstSchemaENS_4TextILm0EEES4_S4_");
extern "C" void agiru_unlinked_706() { Unlinked("agiru::System::Xml::XmlValidationImpl_Codeunit::ValidateAgainstSchema(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_707() asm("_ZN5agiru6System3Xml26XmlValidationImpl_Codeunit21ValidateAgainstSchemaENS_8InStreamES3_NS_4TextILm0EEE");
extern "C" void agiru_unlinked_707() { Unlinked("agiru::System::Xml::XmlValidationImpl_Codeunit::ValidateAgainstSchema(agiru::InStream, agiru::InStream, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_708() asm("_ZN5agiru6System3Xml26XmlValidationImpl_Codeunit21ValidateAgainstSchemaEv");
extern "C" void agiru_unlinked_708() { Unlinked("agiru::System::Xml::XmlValidationImpl_Codeunit::ValidateAgainstSchema()"); }
extern "C" void agiru_unlinked_709() asm("_ZN5agiru6System3Xml36DotNet_XslCompiledTransform_Codeunit20XslCompiledTransformEv");
extern "C" void agiru_unlinked_709() { Unlinked("agiru::System::Xml::DotNet_XslCompiledTransform_Codeunit::XslCompiledTransform()"); }
extern "C" void agiru_unlinked_710() asm("_ZN5agiru6System3Xml36DotNet_XslCompiledTransform_Codeunit4LoadERNS1_27DotNet_XmlDocument_CodeunitE");
extern "C" void agiru_unlinked_710() { Unlinked("agiru::System::Xml::DotNet_XslCompiledTransform_Codeunit::Load(agiru::System::Xml::DotNet_XmlDocument_Codeunit&)"); }
extern "C" void agiru_unlinked_711() asm("_ZN5agiru6System3Xml36DotNet_XslCompiledTransform_Codeunit9TransformERNS1_27DotNet_XmlDocument_CodeunitENS1_32DotNet_XsltArgumentList_CodeunitERNS_9OutStreamE");
extern "C" void agiru_unlinked_711() { Unlinked("agiru::System::Xml::DotNet_XslCompiledTransform_Codeunit::Transform(agiru::System::Xml::DotNet_XmlDocument_Codeunit&, agiru::System::Xml::DotNet_XsltArgumentList_Codeunit, agiru::OutStream&)"); }
extern "C" void agiru_unlinked_712() asm("_ZN5agiru6System4Text16ImplementationOfENS1_26BarcodeFontProvider2D_EnumEPNS1_31BarcodeFontProvider2D_InterfaceE");
extern "C" void agiru_unlinked_712() { Unlinked("agiru::System::Text::ImplementationOf(agiru::System::Text::BarcodeFontProvider2D_Enum, agiru::System::Text::BarcodeFontProvider2D_Interface*)"); }
extern "C" void agiru_unlinked_713() asm("_ZN5agiru6System4Text23EntityTextImpl_Codeunit10CanSuggestEv");
extern "C" void agiru_unlinked_713() { Unlinked("agiru::System::Text::EntityTextImpl_Codeunit::CanSuggest()"); }
extern "C" void agiru_unlinked_714() asm("_ZN5agiru6System4Text23EntityTextImpl_Codeunit14GetFeatureNameEv");
extern "C" void agiru_unlinked_714() { Unlinked("agiru::System::Text::EntityTextImpl_Codeunit::GetFeatureName()"); }
extern "C" void agiru_unlinked_715() asm("_ZN5agiru6System4Text23EntityTextImpl_Codeunit16InsertSuggestionEiNS_4GuidENS_4EnumIvEENS_4TextILm0EEE");
extern "C" void agiru_unlinked_715() { Unlinked("agiru::System::Text::EntityTextImpl_Codeunit::InsertSuggestion(int, agiru::Guid, agiru::Enum<void>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_716() asm("_ZN5agiru6System4Text23EntityTextImpl_Codeunit16InsertSuggestionEiNS_4GuidENS_4EnumIvEENS_4TextILm0EEERNS_6absent10EntityTextE");
extern "C" void agiru_unlinked_716() { Unlinked("agiru::System::Text::EntityTextImpl_Codeunit::InsertSuggestion(int, agiru::Guid, agiru::Enum<void>, agiru::Text<0ul>, agiru::absent::EntityText&)"); }
extern "C" void agiru_unlinked_717() asm("_ZN5agiru6System4Text23EntityTextImpl_Codeunit18GenerateSuggestionENS_10DictionaryINS_4TextILm0EEES5_EENS_4EnumINS1_19EntityTextTone_EnumEEENS7_INS1_21EntityTextFormat_EnumEEENS7_INS1_23EntityTextEmphasis_EnumEEENS_10ModuleInfoE");
extern "C" void agiru_unlinked_717() { Unlinked("agiru::System::Text::EntityTextImpl_Codeunit::GenerateSuggestion(agiru::Dictionary<agiru::Text<0ul>, agiru::Text<0ul> >, agiru::Enum<agiru::System::Text::EntityTextTone_Enum>, agiru::Enum<agiru::System::Text::EntityTextFormat_Enum>, agiru::Enum<agiru::System::Text::EntityTextEmphasis_Enum>, agiru::ModuleInfo)"); }
extern "C" void agiru_unlinked_718() asm("_ZN5agiru6System4Text23EntityTextImpl_Codeunit26SetEntityTextAuthorizationENS_4TextILm0EEES4_NS_10SecretTextE");
extern "C" void agiru_unlinked_718() { Unlinked("agiru::System::Text::EntityTextImpl_Codeunit::SetEntityTextAuthorization(agiru::Text<0ul>, agiru::Text<0ul>, agiru::SecretText)"); }
extern "C" void agiru_unlinked_719() asm("_ZN5agiru6System4Text23EntityTextImpl_Codeunit7GetTextERNS_6absent10EntityTextE");
extern "C" void agiru_unlinked_719() { Unlinked("agiru::System::Text::EntityTextImpl_Codeunit::GetText(agiru::absent::EntityText&)"); }
extern "C" void agiru_unlinked_720() asm("_ZN5agiru6System4Text23EntityTextImpl_Codeunit7GetTextEiNS_4GuidENS_4EnumIvEE");
extern "C" void agiru_unlinked_720() { Unlinked("agiru::System::Text::EntityTextImpl_Codeunit::GetText(int, agiru::Guid, agiru::Enum<void>)"); }
extern "C" void agiru_unlinked_721() asm("_ZN5agiru6System4Text23EntityTextImpl_Codeunit7SetTextERNS_6absent10EntityTextENS_4TextILm0EEE");
extern "C" void agiru_unlinked_721() { Unlinked("agiru::System::Text::EntityTextImpl_Codeunit::SetText(agiru::absent::EntityText&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_722() asm("_ZN5agiru6System4Text23EntityTextImpl_Codeunit9IsEnabledEb");
extern "C" void agiru_unlinked_722() { Unlinked("agiru::System::Text::EntityTextImpl_Codeunit::IsEnabled(bool)"); }
extern "C" void agiru_unlinked_723() asm("_ZN5agiru6System4Text7CloneOfENS1_26BarcodeFontProvider2D_EnumEPNS1_31BarcodeFontProvider2D_InterfaceE");
extern "C" void agiru_unlinked_723() { Unlinked("agiru::System::Text::CloneOf(agiru::System::Text::BarcodeFontProvider2D_Enum, agiru::System::Text::BarcodeFontProvider2D_Interface*)"); }
extern "C" void agiru_unlinked_724() asm("_ZN5agiru6System5Azure8Identity26AzureADTenantImpl_Codeunit14GetAadTenantIdEv");
extern "C" void agiru_unlinked_724() { Unlinked("agiru::System::Azure::Identity::AzureADTenantImpl_Codeunit::GetAadTenantId()"); }
extern "C" void agiru_unlinked_725() asm("_ZN5agiru6System5Azure8Identity26AzureADTenantImpl_Codeunit16IsVerifiedDomainENS_4TextILm0EEE");
extern "C" void agiru_unlinked_725() { Unlinked("agiru::System::Azure::Identity::AzureADTenantImpl_Codeunit::IsVerifiedDomain(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_726() asm("_ZN5agiru6System5Azure8Identity26AzureADTenantImpl_Codeunit20GetCountryLetterCodeEv");
extern "C" void agiru_unlinked_726() { Unlinked("agiru::System::Azure::Identity::AzureADTenantImpl_Codeunit::GetCountryLetterCode()"); }
extern "C" void agiru_unlinked_727() asm("_ZN5agiru6System5Azure8Identity26AzureADTenantImpl_Codeunit20GetPreferredLanguageEv");
extern "C" void agiru_unlinked_727() { Unlinked("agiru::System::Azure::Identity::AzureADTenantImpl_Codeunit::GetPreferredLanguage()"); }
extern "C" void agiru_unlinked_728() asm("_ZN5agiru6System5Azure8Identity26AzureADTenantImpl_Codeunit22GetAadTenantDomainNameEv");
extern "C" void agiru_unlinked_728() { Unlinked("agiru::System::Azure::Identity::AzureADTenantImpl_Codeunit::GetAadTenantDomainName()"); }
extern "C" void agiru_unlinked_729() asm("_ZN5agiru6System5Azure8Identity26AzureADTenantImpl_Codeunit25GetPowerPlatformTenantURLEv");
extern "C" void agiru_unlinked_729() { Unlinked("agiru::System::Azure::Identity::AzureADTenantImpl_Codeunit::GetPowerPlatformTenantURL()"); }
extern "C" void agiru_unlinked_730() asm("_ZN5agiru6System5Azure8Identity28AzureADUserUpdateWizard_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_730() { Unlinked("agiru::System::Azure::Identity::AzureADUserUpdateWizard_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_731() asm("_ZN5agiru6System5Azure8Identity28AzureADUserUpdateWizard_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_731() { Unlinked("agiru::System::Azure::Identity::AzureADUserUpdateWizard_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_732() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit10SendOutboxERNS1_17EmailOutbox_TableE");
extern "C" void agiru_unlinked_732() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::SendOutbox(agiru::System::Email::EmailOutbox_Table&)"); }
extern "C" void agiru_unlinked_733() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit12CreateOutboxERNS1_17EmailOutbox_TableE");
extern "C" void agiru_unlinked_733() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::CreateOutbox(agiru::System::Email::EmailOutbox_Table&)"); }
extern "C" void agiru_unlinked_734() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit12DiscardEmailERNS1_17EmailOutbox_TableEb");
extern "C" void agiru_unlinked_734() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::DiscardEmail(agiru::System::Email::EmailOutbox_Table&, bool)"); }
extern "C" void agiru_unlinked_735() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit15GetEmailAccountENS1_17EmailOutbox_TableERNS1_18EmailAccount_TableE");
extern "C" void agiru_unlinked_735() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::GetEmailAccount(agiru::System::Email::EmailOutbox_Table, agiru::System::Email::EmailAccount_Table&)"); }
extern "C" void agiru_unlinked_736() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit15GetEmailMessageERNS1_17EmailOutbox_TableERNS1_25EmailMessageImpl_CodeunitE");
extern "C" void agiru_unlinked_736() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::GetEmailMessage(agiru::System::Email::EmailOutbox_Table&, agiru::System::Email::EmailMessageImpl_Codeunit&)"); }
extern "C" void agiru_unlinked_737() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit16CheckPermissionsENS1_17EmailOutbox_TableE");
extern "C" void agiru_unlinked_737() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::CheckPermissions(agiru::System::Email::EmailOutbox_Table)"); }
extern "C" void agiru_unlinked_738() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit16LoadWordTemplateENS1_25EmailMessageImpl_CodeunitENS_4GuidE");
extern "C" void agiru_unlinked_738() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::LoadWordTemplate(agiru::System::Email::EmailMessageImpl_Codeunit, agiru::Guid)"); }
extern "C" void agiru_unlinked_739() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit16LookupRecipientsENS_4GuidERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_739() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::LookupRecipients(agiru::Guid, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_740() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit16OpenWithScenarioENS1_17EmailOutbox_TableEbNS_4EnumINS1_18EmailScenario_EnumEEE");
extern "C" void agiru_unlinked_740() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::OpenWithScenario(agiru::System::Email::EmailOutbox_Table, bool, agiru::Enum<agiru::System::Email::EmailScenario_Enum>)"); }
extern "C" void agiru_unlinked_741() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit17ValidateEmailDataENS_4TextILm0EEERNS1_25EmailMessageImpl_CodeunitE");
extern "C" void agiru_unlinked_741() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::ValidateEmailData(agiru::Text<0ul>, agiru::System::Email::EmailMessageImpl_Codeunit&)"); }
extern "C" void agiru_unlinked_742() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit18ChangeEmailAccountERNS1_17EmailOutbox_TableERNS1_18EmailAccount_TableE");
extern "C" void agiru_unlinked_742() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::ChangeEmailAccount(agiru::System::Email::EmailOutbox_Table&, agiru::System::Email::EmailAccount_Table&)"); }
extern "C" void agiru_unlinked_743() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit18DownloadAttachmentENS_4GuidENS_4TextILm0EEE");
extern "C" void agiru_unlinked_743() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::DownloadAttachment(agiru::Guid, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_744() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit19DownloadAttachmentsENS_10DictionaryINS_4GuidENS_4TextILm0EEEEES6_");
extern "C" void agiru_unlinked_744() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::DownloadAttachments(agiru::Dictionary<agiru::Guid, agiru::Text<0ul> >, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_745() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit20VerifyRelatedRecordsENS_4GuidE");
extern "C" void agiru_unlinked_745() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::VerifyRelatedRecords(agiru::Guid)"); }
extern "C" void agiru_unlinked_746() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit21GetRelatedAttachmentsENS_4GuidERNS1_28EmailRelatedAttachment_TableE");
extern "C" void agiru_unlinked_746() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::GetRelatedAttachments(agiru::Guid, agiru::System::Email::EmailRelatedAttachment_Table&)"); }
extern "C" void agiru_unlinked_747() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit26PopulateRelatedRecordCacheENS_4GuidE");
extern "C" void agiru_unlinked_747() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::PopulateRelatedRecordCache(agiru::Guid)"); }
extern "C" void agiru_unlinked_748() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit4OpenENS1_17EmailOutbox_TableEb");
extern "C" void agiru_unlinked_748() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::Open(agiru::System::Email::EmailOutbox_Table, bool)"); }
extern "C" void agiru_unlinked_749() asm("_ZN5agiru6System5Email20EmailEditor_Codeunit8SetAsNewEv");
extern "C" void agiru_unlinked_749() { Unlinked("agiru::System::Email::EmailEditor_Codeunit::SetAsNew()"); }
extern "C" void agiru_unlinked_750() asm("_ZN5agiru6System5Email21EmailAttachments_Page12UpdateValuesENS1_25EmailMessageImpl_CodeunitEb");
extern "C" void agiru_unlinked_750() { Unlinked("agiru::System::Email::EmailAttachments_Page::UpdateValues(agiru::System::Email::EmailMessageImpl_Codeunit, bool)"); }
extern "C" void agiru_unlinked_751() asm("_ZN5agiru6System5Email21EmailAttachments_Page14OnActionDeleteEv");
extern "C" void agiru_unlinked_751() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnActionDelete()"); }
extern "C" void agiru_unlinked_752() asm("_ZN5agiru6System5Email21EmailAttachments_Page15OnEnabledDeleteEv");
extern "C" void agiru_unlinked_752() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnEnabledDelete()"); }
extern "C" void agiru_unlinked_753() asm("_ZN5agiru6System5Email21EmailAttachments_Page15OnVisibleDeleteEv");
extern "C" void agiru_unlinked_753() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnVisibleDelete()"); }
extern "C" void agiru_unlinked_754() asm("_ZN5agiru6System5Email21EmailAttachments_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_754() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_755() asm("_ZN5agiru6System5Email21EmailAttachments_Page19OnDrillDownFileNameEv");
extern "C" void agiru_unlinked_755() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnDrillDownFileName()"); }
extern "C" void agiru_unlinked_756() asm("_ZN5agiru6System5Email21EmailAttachments_Page19UpdateEmailScenarioENS_4EnumINS1_18EmailScenario_EnumEEE");
extern "C" void agiru_unlinked_756() { Unlinked("agiru::System::Email::EmailAttachments_Page::UpdateEmailScenario(agiru::Enum<agiru::System::Email::EmailScenario_Enum>)"); }
extern "C" void agiru_unlinked_757() asm("_ZN5agiru6System5Email21EmailAttachments_Page20OnActionWordTemplateEv");
extern "C" void agiru_unlinked_757() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnActionWordTemplate()"); }
extern "C" void agiru_unlinked_758() asm("_ZN5agiru6System5Email21EmailAttachments_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_758() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_759() asm("_ZN5agiru6System5Email21EmailAttachments_Page21OnVisibleWordTemplateEv");
extern "C" void agiru_unlinked_759() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnVisibleWordTemplate()"); }
extern "C" void agiru_unlinked_760() asm("_ZN5agiru6System5Email21EmailAttachments_Page22OnActionEditInOneDriveEv");
extern "C" void agiru_unlinked_760() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnActionEditInOneDrive()"); }
extern "C" void agiru_unlinked_761() asm("_ZN5agiru6System5Email21EmailAttachments_Page23OnVisibleEditInOneDriveEv");
extern "C" void agiru_unlinked_761() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnVisibleEditInOneDrive()"); }
extern "C" void agiru_unlinked_762() asm("_ZN5agiru6System5Email21EmailAttachments_Page23OnVisibleUploadMultipleEv");
extern "C" void agiru_unlinked_762() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnVisibleUploadMultiple()"); }
extern "C" void agiru_unlinked_763() asm("_ZN5agiru6System5Email21EmailAttachments_Page25OnActionSourceAttachmentsEv");
extern "C" void agiru_unlinked_763() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnActionSourceAttachments()"); }
extern "C" void agiru_unlinked_764() asm("_ZN5agiru6System5Email21EmailAttachments_Page26OnActionUploadFromScenarioEv");
extern "C" void agiru_unlinked_764() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnActionUploadFromScenario()"); }
extern "C" void agiru_unlinked_765() asm("_ZN5agiru6System5Email21EmailAttachments_Page26OnVisibleSourceAttachmentsEv");
extern "C" void agiru_unlinked_765() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnVisibleSourceAttachments()"); }
extern "C" void agiru_unlinked_766() asm("_ZN5agiru6System5Email21EmailAttachments_Page27OnVisibleUploadFromScenarioEv");
extern "C" void agiru_unlinked_766() { Unlinked("agiru::System::Email::EmailAttachments_Page::OnVisibleUploadFromScenario()"); }
extern "C" void agiru_unlinked_767() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit10GetSubjectEv");
extern "C" void agiru_unlinked_767() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::GetSubject()"); }
extern "C" void agiru_unlinked_768() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit10MarkAsReadEv");
extern "C" void agiru_unlinked_768() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::MarkAsRead()"); }
extern "C" void agiru_unlinked_769() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit10SetSubjectENS_4TextILm0EEE");
extern "C" void agiru_unlinked_769() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::SetSubject(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_770() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit11CreateReplyENS_4ListINS_4TextILm0EEEEES5_S5_bS5_S6_S6_");
extern "C" void agiru_unlinked_770() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::CreateReply(agiru::List<agiru::Text<0ul> >, agiru::Text<0ul>, agiru::Text<0ul>, bool, agiru::Text<0ul>, agiru::List<agiru::Text<0ul> >, agiru::List<agiru::Text<0ul> >)"); }
extern "C" void agiru_unlinked_771() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit11CreateReplyENS_4TextILm0EEES4_S4_bS4_");
extern "C" void agiru_unlinked_771() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::CreateReply(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>, bool, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_772() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit12AddRecipientENS_4EnumINS1_23EmailRecipientType_EnumEEENS_4TextILm0EEE");
extern "C" void agiru_unlinked_772() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::AddRecipient(agiru::Enum<agiru::System::Email::EmailRecipientType_Enum>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_773() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit12AppendToBodyENS_4TextILm0EEE");
extern "C" void agiru_unlinked_773() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::AppendToBody(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_774() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit12FlushHeadersEv");
extern "C" void agiru_unlinked_774() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::FlushHeaders()"); }
extern "C" void agiru_unlinked_775() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit13AddAttachmentENS_4TextILm250EEES4_NS3_ILm0EEE");
extern "C" void agiru_unlinked_775() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::AddAttachment(agiru::Text<250ul>, agiru::Text<250ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_776() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit13AddAttachmentENS_4TextILm250EEES4_NS_8InStreamE");
extern "C" void agiru_unlinked_776() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::AddAttachment(agiru::Text<250ul>, agiru::Text<250ul>, agiru::InStream)"); }
extern "C" void agiru_unlinked_777() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit13AddAttachmentENS_4TextILm250EEES4_NS_8InStreamEbNS3_ILm40EEE");
extern "C" void agiru_unlinked_777() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::AddAttachment(agiru::Text<250ul>, agiru::Text<250ul>, agiru::InStream, bool, agiru::Text<40ul>)"); }
extern "C" void agiru_unlinked_778() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit13GetExternalIdEv");
extern "C" void agiru_unlinked_778() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::GetExternalId()"); }
extern "C" void agiru_unlinked_779() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit13GetRecipientsENS_4EnumINS1_23EmailRecipientType_EnumEEE");
extern "C" void agiru_unlinked_779() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::GetRecipients(agiru::Enum<agiru::System::Email::EmailRecipientType_Enum>)"); }
extern "C" void agiru_unlinked_780() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit13SetRecipientsENS_4EnumINS1_23EmailRecipientType_EnumEEENS_4ListINS_4TextILm0EEEEE");
extern "C" void agiru_unlinked_780() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::SetRecipients(agiru::Enum<agiru::System::Email::EmailRecipientType_Enum>, agiru::List<agiru::Text<0ul> >)"); }
extern "C" void agiru_unlinked_781() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit13SetRecipientsENS_4EnumINS1_23EmailRecipientType_EnumEEENS_4TextILm0EEE");
extern "C" void agiru_unlinked_781() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::SetRecipients(agiru::Enum<agiru::System::Email::EmailRecipientType_Enum>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_782() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit14CreateReplyAllENS_4TextILm0EEES4_bS4_");
extern "C" void agiru_unlinked_782() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::CreateReplyAll(agiru::Text<0ul>, agiru::Text<0ul>, bool, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_783() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit15GetEmailMessageERNS1_18EmailMessage_TableE");
extern "C" void agiru_unlinked_783() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::GetEmailMessage(agiru::System::Email::EmailMessage_Table&)"); }
extern "C" void agiru_unlinked_784() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit15GetNoOfModifiesEv");
extern "C" void agiru_unlinked_784() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::GetNoOfModifies()"); }
extern "C" void agiru_unlinked_785() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit16Attachments_NextEv");
extern "C" void agiru_unlinked_785() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_Next()"); }
extern "C" void agiru_unlinked_786() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit17Attachments_FirstEv");
extern "C" void agiru_unlinked_786() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_First()"); }
extern "C" void agiru_unlinked_787() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit17Attachments_GetIdEv");
extern "C" void agiru_unlinked_787() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_GetId()"); }
extern "C" void agiru_unlinked_788() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit18Attachments_DeleteEb");
extern "C" void agiru_unlinked_788() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_Delete(bool)"); }
extern "C" void agiru_unlinked_789() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit18Attachments_DeleteEv");
extern "C" void agiru_unlinked_789() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_Delete()"); }
extern "C" void agiru_unlinked_790() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit18ValidateRecipientsEv");
extern "C" void agiru_unlinked_790() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::ValidateRecipients()"); }
extern "C" void agiru_unlinked_791() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit19Attachments_GetNameEv");
extern "C" void agiru_unlinked_791() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_GetName()"); }
extern "C" void agiru_unlinked_792() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit19GetRecipientsAsTextENS_4EnumINS1_23EmailRecipientType_EnumEEE");
extern "C" void agiru_unlinked_792() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::GetRecipientsAsText(agiru::Enum<agiru::System::Email::EmailRecipientType_Enum>)"); }
extern "C" void agiru_unlinked_793() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit19IsBodyHTMLFormattedEv");
extern "C" void agiru_unlinked_793() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::IsBodyHTMLFormatted()"); }
extern "C" void agiru_unlinked_794() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit20Attachments_IsInlineEv");
extern "C" void agiru_unlinked_794() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_IsInline()"); }
extern "C" void agiru_unlinked_795() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit20SetBodyHTMLFormattedEb");
extern "C" void agiru_unlinked_795() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::SetBodyHTMLFormatted(bool)"); }
extern "C" void agiru_unlinked_796() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit21Attachments_GetLengthEv");
extern "C" void agiru_unlinked_796() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_GetLength()"); }
extern "C" void agiru_unlinked_797() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit21GetRelatedAttachmentsERNS1_28EmailRelatedAttachment_TableE");
extern "C" void agiru_unlinked_797() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::GetRelatedAttachments(agiru::System::Email::EmailRelatedAttachment_Table&)"); }
extern "C" void agiru_unlinked_798() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit22Attachments_GetContentERNS_8InStreamE");
extern "C" void agiru_unlinked_798() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_GetContent(agiru::InStream&)"); }
extern "C" void agiru_unlinked_799() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit22DeleteOrphanedMessagesENS_4GuidEi");
extern "C" void agiru_unlinked_799() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::DeleteOrphanedMessages(agiru::Guid, int)"); }
extern "C" void agiru_unlinked_800() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit24Attachments_GetContentIdEv");
extern "C" void agiru_unlinked_800() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_GetContentId()"); }
extern "C" void agiru_unlinked_801() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit25Attachments_DeleteContentEb");
extern "C" void agiru_unlinked_801() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_DeleteContent(bool)"); }
extern "C" void agiru_unlinked_802() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit25Attachments_DeleteContentEv");
extern "C" void agiru_unlinked_802() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_DeleteContent()"); }
extern "C" void agiru_unlinked_803() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit26AddAttachmentsFromScenarioERNS1_22EmailAttachments_TableE");
extern "C" void agiru_unlinked_803() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::AddAttachmentsFromScenario(agiru::System::Email::EmailAttachments_Table&)"); }
extern "C" void agiru_unlinked_804() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit26Attachments_GetContentTypeEv");
extern "C" void agiru_unlinked_804() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_GetContentType()"); }
extern "C" void agiru_unlinked_805() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit28Attachments_GetContentBase64Ev");
extern "C" void agiru_unlinked_805() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Attachments_GetContentBase64()"); }
extern "C" void agiru_unlinked_806() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit31DeleteEmailRecipientsIfOrphanedENS_4GuidEi");
extern "C" void agiru_unlinked_806() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::DeleteEmailRecipientsIfOrphaned(agiru::Guid, int)"); }
extern "C" void agiru_unlinked_807() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit3GetENS_4GuidE");
extern "C" void agiru_unlinked_807() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Get(agiru::Guid)"); }
extern "C" void agiru_unlinked_808() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit5GetIdEv");
extern "C" void agiru_unlinked_808() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::GetId()"); }
extern "C" void agiru_unlinked_809() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit6CreateENS_4ListINS_4TextILm0EEEEES5_S5_bS6_S6_b");
extern "C" void agiru_unlinked_809() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Create(agiru::List<agiru::Text<0ul> >, agiru::Text<0ul>, agiru::Text<0ul>, bool, agiru::List<agiru::Text<0ul> >, agiru::List<agiru::Text<0ul> >, bool)"); }
extern "C" void agiru_unlinked_810() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit6CreateENS_4TextILm0EEES4_S4_b");
extern "C" void agiru_unlinked_810() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Create(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>, bool)"); }
extern "C" void agiru_unlinked_811() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit6CreateENS_4TextILm0EEES4_S4_bb");
extern "C" void agiru_unlinked_811() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Create(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Text<0ul>, bool, bool)"); }
extern "C" void agiru_unlinked_812() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit6CreateES2_");
extern "C" void agiru_unlinked_812() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Create(agiru::System::Email::EmailMessageImpl_Codeunit)"); }
extern "C" void agiru_unlinked_813() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit6IsReadEv");
extern "C" void agiru_unlinked_813() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::IsRead()"); }
extern "C" void agiru_unlinked_814() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit6ModifyEv");
extern "C" void agiru_unlinked_814() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::Modify()"); }
extern "C" void agiru_unlinked_815() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit7GetBodyEv");
extern "C" void agiru_unlinked_815() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::GetBody()"); }
extern "C" void agiru_unlinked_816() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit7SetBodyENS_4TextILm0EEE");
extern "C" void agiru_unlinked_816() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::SetBody(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_817() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit9AddHeaderENS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_817() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::AddHeader(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_818() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit9GetHeaderENS_4TextILm0EEERS4_");
extern "C" void agiru_unlinked_818() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::GetHeader(agiru::Text<0ul>, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_819() asm("_ZN5agiru6System5Email25EmailMessageImpl_Codeunit9SetHeaderENS_4TextILm0EEES4_");
extern "C" void agiru_unlinked_819() { Unlinked("agiru::System::Email::EmailMessageImpl_Codeunit::SetHeader(agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_820() asm("_ZN5agiru6System5Email32EmailScenarioAttachImpl_Codeunit13AddAttachmentERNS1_30EmailScenarioAttachments_TableERNS1_22EmailAttachments_TableEi");
extern "C" void agiru_unlinked_820() { Unlinked("agiru::System::Email::EmailScenarioAttachImpl_Codeunit::AddAttachment(agiru::System::Email::EmailScenarioAttachments_Table&, agiru::System::Email::EmailAttachments_Table&, int)"); }
extern "C" void agiru_unlinked_821() asm("_ZN5agiru6System5Email32EmailScenarioAttachImpl_Codeunit22AddAttachmentToMessageERNS1_21EmailMessage_CodeunitENS_4EnumINS1_18EmailScenario_EnumEEE");
extern "C" void agiru_unlinked_821() { Unlinked("agiru::System::Email::EmailScenarioAttachImpl_Codeunit::AddAttachmentToMessage(agiru::System::Email::EmailMessage_Codeunit&, agiru::Enum<agiru::System::Email::EmailScenario_Enum>)"); }
extern "C" void agiru_unlinked_822() asm("_ZN5agiru6System5Email32EmailScenarioAttachImpl_Codeunit24AddAttachmentToScenariosENS1_30EmailScenarioAttachments_TableERNS1_22EmailAttachments_TableERNS1_26EmailAccountScenario_TableE");
extern "C" void agiru_unlinked_822() { Unlinked("agiru::System::Email::EmailScenarioAttachImpl_Codeunit::AddAttachmentToScenarios(agiru::System::Email::EmailScenarioAttachments_Table, agiru::System::Email::EmailAttachments_Table&, agiru::System::Email::EmailAccountScenario_Table&)"); }
extern "C" void agiru_unlinked_823() asm("_ZN5agiru6System5Email32EmailScenarioAttachImpl_Codeunit25DeleteScenarioAttachmentsERNS1_22EmailAttachments_TableERNS1_30EmailScenarioAttachments_TableE");
extern "C" void agiru_unlinked_823() { Unlinked("agiru::System::Email::EmailScenarioAttachImpl_Codeunit::DeleteScenarioAttachments(agiru::System::Email::EmailAttachments_Table&, agiru::System::Email::EmailScenarioAttachments_Table&)"); }
extern "C" void agiru_unlinked_824() asm("_ZN5agiru6System5Email32EmailScenarioAttachImpl_Codeunit26SetEmailScenarioAttachmentERNS1_22EmailAttachments_TableERNS1_30EmailScenarioAttachments_TableE");
extern "C" void agiru_unlinked_824() { Unlinked("agiru::System::Email::EmailScenarioAttachImpl_Codeunit::SetEmailScenarioAttachment(agiru::System::Email::EmailAttachments_Table&, agiru::System::Email::EmailScenarioAttachments_Table&)"); }
extern "C" void agiru_unlinked_825() asm("_ZN5agiru6System5Email32EmailScenarioAttachImpl_Codeunit32SetEmailScenarioAttachmentStatusERNS1_30EmailScenarioAttachments_TableElb");
extern "C" void agiru_unlinked_825() { Unlinked("agiru::System::Email::EmailScenarioAttachImpl_Codeunit::SetEmailScenarioAttachmentStatus(agiru::System::Email::EmailScenarioAttachments_Table&, long, bool)"); }
extern "C" void agiru_unlinked_826() asm("_ZN5agiru6System5Email32EmailScenarioAttachImpl_Codeunit35GetEmailAttachmentsByEmailScenariosERNS1_22EmailAttachments_TableEi");
extern "C" void agiru_unlinked_826() { Unlinked("agiru::System::Email::EmailScenarioAttachImpl_Codeunit::GetEmailAttachmentsByEmailScenarios(agiru::System::Email::EmailAttachments_Table&, int)"); }
extern "C" void agiru_unlinked_827() asm("_ZN5agiru6System6Device31InitServerPrinterTable_Codeunit19ValidatePrinterNameERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_827() { Unlinked("agiru::System::Device::InitServerPrinterTable_Codeunit::ValidatePrinterName(agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_828() asm("_ZN5agiru6System6Device31InitServerPrinterTable_Codeunit38FindClosestMatchToClientDefaultPrinterEi");
extern "C" void agiru_unlinked_828() { Unlinked("agiru::System::Device::InitServerPrinterTable_Codeunit::FindClosestMatchToClientDefaultPrinter(int)"); }
extern "C" void agiru_unlinked_829() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit13SyncAllFieldsEv");
extern "C" void agiru_unlinked_829() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::SyncAllFields()"); }
extern "C" void agiru_unlinked_830() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit16IsSupportedTableEi");
extern "C" void agiru_unlinked_830() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::IsSupportedTable(int)"); }
extern "C" void agiru_unlinked_831() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit16SetFieldToNormalEii");
extern "C" void agiru_unlinked_831() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::SetFieldToNormal(int, int)"); }
extern "C" void agiru_unlinked_832() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit16SetSensitivitiesERNS_6absent15DataSensitivityENS_6OptionIvEE");
extern "C" void agiru_unlinked_832() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::SetSensitivities(agiru::absent::DataSensitivity&, agiru::Option<void>)"); }
extern "C" void agiru_unlinked_833() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit18PopulateFieldValueENS_8FieldRefERNS1_24FieldContentBuffer_TableE");
extern "C" void agiru_unlinked_833() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::PopulateFieldValue(agiru::FieldRef, agiru::System::Privacy::FieldContentBuffer_Table&)"); }
extern "C" void agiru_unlinked_834() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit18SetFieldToPersonalEii");
extern "C" void agiru_unlinked_834() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::SetFieldToPersonal(int, int)"); }
extern "C" void agiru_unlinked_835() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit19SetFieldToSensitiveEii");
extern "C" void agiru_unlinked_835() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::SetFieldToSensitive(int, int)"); }
extern "C" void agiru_unlinked_836() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit21GetLegalDisclaimerTxtEv");
extern "C" void agiru_unlinked_836() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::GetLegalDisclaimerTxt()"); }
extern "C" void agiru_unlinked_837() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit22AreAllFieldsClassifiedEv");
extern "C" void agiru_unlinked_837() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::AreAllFieldsClassified()"); }
extern "C" void agiru_unlinked_838() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit22SetTableFieldsToNormalEi");
extern "C" void agiru_unlinked_838() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::SetTableFieldsToNormal(int)"); }
extern "C" void agiru_unlinked_839() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit28PopulateDataSensitivityTableEv");
extern "C" void agiru_unlinked_839() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::PopulateDataSensitivityTable()"); }
extern "C" void agiru_unlinked_840() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit29InsertDataSensitivityForFieldEiiNS_6OptionIvEE");
extern "C" void agiru_unlinked_840() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::InsertDataSensitivityForField(int, int, agiru::Option<void>)"); }
extern "C" void agiru_unlinked_841() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit29SetFieldToCompanyConfidentialEii");
extern "C" void agiru_unlinked_841() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::SetFieldToCompanyConfidential(int, int)"); }
extern "C" void agiru_unlinked_842() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit30GetDataSensitivityOptionStringEv");
extern "C" void agiru_unlinked_842() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::GetDataSensitivityOptionString()"); }
extern "C" void agiru_unlinked_843() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit38RunDataClassificationWorksheetForTableEi");
extern "C" void agiru_unlinked_843() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::RunDataClassificationWorksheetForTable(int)"); }
extern "C" void agiru_unlinked_844() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit39IsDataSensitivityEmptyForCurrentCompanyEv");
extern "C" void agiru_unlinked_844() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::IsDataSensitivityEmptyForCurrentCompany()"); }
extern "C" void agiru_unlinked_845() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit55RunDataClassificationWorksheetForTableWhoseNameContainsENS_4TextILm0EEE");
extern "C" void agiru_unlinked_845() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::RunDataClassificationWorksheetForTableWhoseNameContains(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_846() asm("_ZN5agiru6System7Privacy34DataClassificationMgtImpl_Codeunit64RunDataClassificationWorksheetForPersonalAndSensitiveDataInTableEi");
extern "C" void agiru_unlinked_846() { Unlinked("agiru::System::Privacy::DataClassificationMgtImpl_Codeunit::RunDataClassificationWorksheetForPersonalAndSensitiveDataInTable(int)"); }
extern "C" void agiru_unlinked_847() asm("_ZN5agiru6System7Tooling18AddPageFields_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_847() { Unlinked("agiru::System::Tooling::AddPageFields_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_848() asm("_ZN5agiru6System7Tooling27ProfilingFullTimeChart_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_848() { Unlinked("agiru::System::Tooling::ProfilingFullTimeChart_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_849() asm("_ZN5agiru6System7Tooling27ProfilingFullTimeChart_Page10UpdateDataEv");
extern "C" void agiru_unlinked_849() { Unlinked("agiru::System::Tooling::ProfilingFullTimeChart_Page::UpdateData()"); }
extern "C" void agiru_unlinked_850() asm("_ZN5agiru6System7Tooling27ProfilingSelfTimeChart_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_850() { Unlinked("agiru::System::Tooling::ProfilingSelfTimeChart_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_851() asm("_ZN5agiru6System7Tooling27ProfilingSelfTimeChart_Page10UpdateDataEv");
extern "C" void agiru_unlinked_851() { Unlinked("agiru::System::Tooling::ProfilingSelfTimeChart_Page::UpdateData()"); }
extern "C" void agiru_unlinked_852() asm("_ZN5agiru6System7Tooling33SamplingPerfProfilerImpl_Codeunit12DownloadDataENS_4TextILm0EEENS_8InStreamE");
extern "C" void agiru_unlinked_852() { Unlinked("agiru::System::Tooling::SamplingPerfProfilerImpl_Codeunit::DownloadData(agiru::Text<0ul>, agiru::InStream)"); }
extern "C" void agiru_unlinked_853() asm("_ZN5agiru6System7Tooling33SamplingPerfProfilerImpl_Codeunit17GetProfilingNodesERNS1_19ProfilingNode_TableE");
extern "C" void agiru_unlinked_853() { Unlinked("agiru::System::Tooling::SamplingPerfProfilerImpl_Codeunit::GetProfilingNodes(agiru::System::Tooling::ProfilingNode_Table&)"); }
extern "C" void agiru_unlinked_854() asm("_ZN5agiru6System7Tooling33SamplingPerfProfilerImpl_Codeunit20GetProfilingCallTreeERNS1_19ProfilingNode_TableE");
extern "C" void agiru_unlinked_854() { Unlinked("agiru::System::Tooling::SamplingPerfProfilerImpl_Codeunit::GetProfilingCallTree(agiru::System::Tooling::ProfilingNode_Table&)"); }
extern "C" void agiru_unlinked_855() asm("_ZN5agiru6System7Tooling33SamplingPerfProfilerImpl_Codeunit21IsRecordingInProgressEv");
extern "C" void agiru_unlinked_855() { Unlinked("agiru::System::Tooling::SamplingPerfProfilerImpl_Codeunit::IsRecordingInProgress()"); }
extern "C" void agiru_unlinked_856() asm("_ZN5agiru6System7Tooling33SamplingPerfProfilerImpl_Codeunit4StopEv");
extern "C" void agiru_unlinked_856() { Unlinked("agiru::System::Tooling::SamplingPerfProfilerImpl_Codeunit::Stop()"); }
extern "C" void agiru_unlinked_857() asm("_ZN5agiru6System7Tooling33SamplingPerfProfilerImpl_Codeunit5StartENS_4EnumINS1_21SamplingInterval_EnumEEE");
extern "C" void agiru_unlinked_857() { Unlinked("agiru::System::Tooling::SamplingPerfProfilerImpl_Codeunit::Start(agiru::Enum<agiru::System::Tooling::SamplingInterval_Enum>)"); }
extern "C" void agiru_unlinked_858() asm("_ZN5agiru6System7Tooling33SamplingPerfProfilerImpl_Codeunit5StartEv");
extern "C" void agiru_unlinked_858() { Unlinked("agiru::System::Tooling::SamplingPerfProfilerImpl_Codeunit::Start()"); }
extern "C" void agiru_unlinked_859() asm("_ZN5agiru6System7Tooling33SamplingPerfProfilerImpl_Codeunit7GetDataEv");
extern "C" void agiru_unlinked_859() { Unlinked("agiru::System::Tooling::SamplingPerfProfilerImpl_Codeunit::GetData()"); }
extern "C" void agiru_unlinked_860() asm("_ZN5agiru6System7Tooling33SamplingPerfProfilerImpl_Codeunit7SetDataENS_8InStreamE");
extern "C" void agiru_unlinked_860() { Unlinked("agiru::System::Tooling::SamplingPerfProfilerImpl_Codeunit::SetData(agiru::InStream)"); }
extern "C" void agiru_unlinked_861() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit11FilterUsersERNS_6absent27PerformanceProfileSchedulerENS_4GuidEb");
extern "C" void agiru_unlinked_861() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::FilterUsers(agiru::absent::PerformanceProfileScheduler&, agiru::Guid, bool)"); }
extern "C" void agiru_unlinked_862() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit11FilterUsersERNS_9RecordRefENS_4GuidEb");
extern "C" void agiru_unlinked_862() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::FilterUsers(agiru::RecordRef&, agiru::Guid, bool)"); }
extern "C" void agiru_unlinked_863() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit16InitializeFieldsERNS_6absent27PerformanceProfileSchedulerERNS_4EnumINS1_28PerfProfileActivityType_EnumEEE");
extern "C" void agiru_unlinked_863() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::InitializeFields(agiru::absent::PerformanceProfileScheduler&, agiru::Enum<agiru::System::Tooling::PerfProfileActivityType_Enum>&)"); }
extern "C" void agiru_unlinked_864() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit17ValidateThresholdERNS_6absent27PerformanceProfileSchedulerE");
extern "C" void agiru_unlinked_864() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::ValidateThreshold(agiru::absent::PerformanceProfileScheduler&)"); }
extern "C" void agiru_unlinked_865() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit18GetRetentionPeriodEv");
extern "C" void agiru_unlinked_865() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::GetRetentionPeriod()"); }
extern "C" void agiru_unlinked_866() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit18IsProfilingEnabledERNS_4GuidE");
extern "C" void agiru_unlinked_866() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::IsProfilingEnabled(agiru::Guid&)"); }
extern "C" void agiru_unlinked_867() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit19MapRecordToUserNameENS_6absent27PerformanceProfileSchedulerE");
extern "C" void agiru_unlinked_867() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::MapRecordToUserName(agiru::absent::PerformanceProfileScheduler)"); }
extern "C" void agiru_unlinked_868() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit23MapActivityTypeToRecordERNS_6absent27PerformanceProfileSchedulerENS_4EnumINS1_28PerfProfileActivityType_EnumEEE");
extern "C" void agiru_unlinked_868() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::MapActivityTypeToRecord(agiru::absent::PerformanceProfileScheduler&, agiru::Enum<agiru::System::Tooling::PerfProfileActivityType_Enum>)"); }
extern "C" void agiru_unlinked_869() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit23MapRecordToActivityTypeENS_6absent27PerformanceProfileSchedulerERNS_4EnumINS1_28PerfProfileActivityType_EnumEEE");
extern "C" void agiru_unlinked_869() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::MapRecordToActivityType(agiru::absent::PerformanceProfileScheduler, agiru::Enum<agiru::System::Tooling::PerfProfileActivityType_Enum>&)"); }
extern "C" void agiru_unlinked_870() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit33ValidatePerformanceProfileEndTimeENS_6absent27PerformanceProfileSchedulerE");
extern "C" void agiru_unlinked_870() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::ValidatePerformanceProfileEndTime(agiru::absent::PerformanceProfileScheduler)"); }
extern "C" void agiru_unlinked_871() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit35ValidatePerformanceProfileSchedulerENS_6absent27PerformanceProfileSchedulerENS_4EnumINS1_28PerfProfileActivityType_EnumEEE");
extern "C" void agiru_unlinked_871() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::ValidatePerformanceProfileScheduler(agiru::absent::PerformanceProfileScheduler, agiru::Enum<agiru::System::Tooling::PerfProfileActivityType_Enum>)"); }
extern "C" void agiru_unlinked_872() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit35ValidateScheduleCreationPermissionsENS_4GuidES3_");
extern "C" void agiru_unlinked_872() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::ValidateScheduleCreationPermissions(agiru::Guid, agiru::Guid)"); }
extern "C" void agiru_unlinked_873() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit40ValidatePerformanceProfileSchedulerDatesENS_6absent27PerformanceProfileSchedulerENS_8DurationE");
extern "C" void agiru_unlinked_873() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::ValidatePerformanceProfileSchedulerDates(agiru::absent::PerformanceProfileScheduler, agiru::Duration)"); }
extern "C" void agiru_unlinked_874() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit48ValidatePerformanceProfileSchedulerDatesRelationENS_6absent27PerformanceProfileSchedulerE");
extern "C" void agiru_unlinked_874() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::ValidatePerformanceProfileSchedulerDatesRelation(agiru::absent::PerformanceProfileScheduler)"); }
extern "C" void agiru_unlinked_875() asm("_ZN5agiru6System7Tooling34ScheduledPerfProfilerImpl_Codeunit9GetStatusENS_6absent27PerformanceProfileSchedulerE");
extern "C" void agiru_unlinked_875() { Unlinked("agiru::System::Tooling::ScheduledPerfProfilerImpl_Codeunit::GetStatus(agiru::absent::PerformanceProfileScheduler)"); }
extern "C" void agiru_unlinked_876() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit10ToDateTimeEv");
extern "C" void agiru_unlinked_876() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::ToDateTime()"); }
extern "C" void agiru_unlinked_877() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit11MillisecondEv");
extern "C" void agiru_unlinked_877() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::Millisecond()"); }
extern "C" void agiru_unlinked_878() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit11SetDateTimeENS_6dotnet8DateTimeE");
extern "C" void agiru_unlinked_878() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::SetDateTime(agiru::dotnet::DateTime)"); }
extern "C" void agiru_unlinked_879() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit13TryParseExactENS_4TextILm0EEES4_NS0_13Globalization27DotNet_CultureInfo_CodeunitENS1_30DotNet_DateTimeStyles_CodeunitE");
extern "C" void agiru_unlinked_879() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::TryParseExact(agiru::Text<0ul>, agiru::Text<0ul>, agiru::System::Globalization::DotNet_CultureInfo_Codeunit, agiru::System::DateTime::DotNet_DateTimeStyles_Codeunit)"); }
extern "C" void agiru_unlinked_880() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit3DayEv");
extern "C" void agiru_unlinked_880() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::Day()"); }
extern "C" void agiru_unlinked_881() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit4HourEv");
extern "C" void agiru_unlinked_881() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::Hour()"); }
extern "C" void agiru_unlinked_882() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit4YearEv");
extern "C" void agiru_unlinked_882() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::Year()"); }
extern "C" void agiru_unlinked_883() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit5MonthEv");
extern "C" void agiru_unlinked_883() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::Month()"); }
extern "C" void agiru_unlinked_884() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit6MinuteEv");
extern "C" void agiru_unlinked_884() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::Minute()"); }
extern "C" void agiru_unlinked_885() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit6SecondEv");
extern "C" void agiru_unlinked_885() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::Second()"); }
extern "C" void agiru_unlinked_886() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit8DateTimeEiii");
extern "C" void agiru_unlinked_886() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::DateTime(int, int, int)"); }
extern "C" void agiru_unlinked_887() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit8ToStringENS1_34DotNet_DateTimeFormatInfo_CodeunitE");
extern "C" void agiru_unlinked_887() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::ToString(agiru::System::DateTime::DotNet_DateTimeFormatInfo_Codeunit)"); }
extern "C" void agiru_unlinked_888() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit8ToStringENS_4TextILm0EEENS1_34DotNet_DateTimeFormatInfo_CodeunitE");
extern "C" void agiru_unlinked_888() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::ToString(agiru::Text<0ul>, agiru::System::DateTime::DotNet_DateTimeFormatInfo_Codeunit)"); }
extern "C" void agiru_unlinked_889() asm("_ZN5agiru6System8DateTime24DotNet_DateTime_Codeunit8TryParseENS_4TextILm0EEENS0_13Globalization27DotNet_CultureInfo_CodeunitENS1_30DotNet_DateTimeStyles_CodeunitE");
extern "C" void agiru_unlinked_889() { Unlinked("agiru::System::DateTime::DotNet_DateTime_Codeunit::TryParse(agiru::Text<0ul>, agiru::System::Globalization::DotNet_CultureInfo_Codeunit, agiru::System::DateTime::DotNet_DateTimeStyles_Codeunit)"); }
extern "C" void agiru_unlinked_890() asm("_ZN5agiru6System8Feedback25OnboardingSignal_Codeunit29CheckAndEmitOnboardingSignalsEv");
extern "C" void agiru_unlinked_890() { Unlinked("agiru::System::Feedback::OnboardingSignal_Codeunit::CheckAndEmitOnboardingSignals()"); }
extern "C" void agiru_unlinked_891() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit13SetSigningKeyENS2_21SignatureKey_CodeunitE");
extern "C" void agiru_unlinked_891() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::SetSigningKey(agiru::System::Security::Encryption::SignatureKey_Codeunit)"); }
extern "C" void agiru_unlinked_892() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit13SetSigningKeyENS_10SecretTextE");
extern "C" void agiru_unlinked_892() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::SetSigningKey(agiru::SecretText)"); }
extern "C" void agiru_unlinked_893() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit13SetSigningKeyENS_10SecretTextENS_4EnumINS2_23SignatureAlgorithm_EnumEEE");
extern "C" void agiru_unlinked_893() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::SetSigningKey(agiru::SecretText, agiru::Enum<agiru::System::Security::Encryption::SignatureAlgorithm_Enum>)"); }
extern "C" void agiru_unlinked_894() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit14CheckSignatureENS_10SecretTextE");
extern "C" void agiru_unlinked_894() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::CheckSignature(agiru::SecretText)"); }
extern "C" void agiru_unlinked_895() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit14CheckSignatureENS_4TextILm0EEENS_10SecretTextEb");
extern "C" void agiru_unlinked_895() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::CheckSignature(agiru::Text<0ul>, agiru::SecretText, bool)"); }
extern "C" void agiru_unlinked_896() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit14CheckSignatureEv");
extern "C" void agiru_unlinked_896() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::CheckSignature()"); }
extern "C" void agiru_unlinked_897() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit15SetDigestMethodENS_4TextILm0EEE");
extern "C" void agiru_unlinked_897() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::SetDigestMethod(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_898() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit16ComputeSignatureEv");
extern "C" void agiru_unlinked_898() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::ComputeSignature()"); }
extern "C" void agiru_unlinked_899() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit16GetXmlDsigDSAUrlEv");
extern "C" void agiru_unlinked_899() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigDSAUrl()"); }
extern "C" void agiru_unlinked_900() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit17GetXmlDsigSHA1UrlEv");
extern "C" void agiru_unlinked_900() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigSHA1Url()"); }
extern "C" void agiru_unlinked_901() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit17InitializeKeyInfoEv");
extern "C" void agiru_unlinked_901() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::InitializeKeyInfo()"); }
extern "C" void agiru_unlinked_902() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit18SetSignatureMethodENS_4TextILm0EEE");
extern "C" void agiru_unlinked_902() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::SetSignatureMethod(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_903() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit19GetXmlDsigSHA256UrlEv");
extern "C" void agiru_unlinked_903() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigSHA256Url()"); }
extern "C" void agiru_unlinked_904() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit19GetXmlDsigSHA384UrlEv");
extern "C" void agiru_unlinked_904() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigSHA384Url()"); }
extern "C" void agiru_unlinked_905() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit19GetXmlDsigSHA512UrlEv");
extern "C" void agiru_unlinked_905() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigSHA512Url()"); }
extern "C" void agiru_unlinked_906() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit19InitializeReferenceENS_4TextILm0EEE");
extern "C" void agiru_unlinked_906() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::InitializeReference(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_907() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit19InitializeSignedXmlENS_10XmlElementE");
extern "C" void agiru_unlinked_907() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::InitializeSignedXml(agiru::XmlElement)"); }
extern "C" void agiru_unlinked_908() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit19InitializeSignedXmlENS_11XmlDocumentE");
extern "C" void agiru_unlinked_908() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::InitializeSignedXml(agiru::XmlDocument)"); }
extern "C" void agiru_unlinked_909() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit20GetXmlDsigRSASHA1UrlEv");
extern "C" void agiru_unlinked_909() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigRSASHA1Url()"); }
extern "C" void agiru_unlinked_910() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit20InitializeDataObjectEv");
extern "C" void agiru_unlinked_910() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::InitializeDataObject()"); }
extern "C" void agiru_unlinked_911() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit21GetXmlDsigHMACSHA1UrlEv");
extern "C" void agiru_unlinked_911() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigHMACSHA1Url()"); }
extern "C" void agiru_unlinked_912() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit22GetXmlDsigRSASHA256UrlEv");
extern "C" void agiru_unlinked_912() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigRSASHA256Url()"); }
extern "C" void agiru_unlinked_913() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit22GetXmlDsigRSASHA384UrlEv");
extern "C" void agiru_unlinked_913() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigRSASHA384Url()"); }
extern "C" void agiru_unlinked_914() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit22GetXmlDsigRSASHA512UrlEv");
extern "C" void agiru_unlinked_914() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigRSASHA512Url()"); }
extern "C" void agiru_unlinked_915() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit23AddReferenceToSignedXMLEv");
extern "C" void agiru_unlinked_915() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::AddReferenceToSignedXML()"); }
extern "C" void agiru_unlinked_916() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit25SetCanonicalizationMethodENS_4TextILm0EEE");
extern "C" void agiru_unlinked_916() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::SetCanonicalizationMethod(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_917() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit26GetXmlDsigC14NTransformUrlEv");
extern "C" void agiru_unlinked_917() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigC14NTransformUrl()"); }
extern "C" void agiru_unlinked_918() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit29GetXmlDsigExcC14NTransformUrlEv");
extern "C" void agiru_unlinked_918() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXmlDsigExcC14NTransformUrl()"); }
extern "C" void agiru_unlinked_919() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit34AddXmlDsigC14NTransformToReferenceEb");
extern "C" void agiru_unlinked_919() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::AddXmlDsigC14NTransformToReference(bool)"); }
extern "C" void agiru_unlinked_920() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit35AddKeyInfoClauseFromX509CertificateENS_4TextILm0EEENS_10SecretTextE");
extern "C" void agiru_unlinked_920() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::AddKeyInfoClauseFromX509Certificate(agiru::Text<0ul>, agiru::SecretText)"); }
extern "C" void agiru_unlinked_921() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit37AddXmlDsigEnvelopedSignatureTransformEv");
extern "C" void agiru_unlinked_921() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::AddXmlDsigEnvelopedSignatureTransform()"); }
extern "C" void agiru_unlinked_922() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit37AddXmlDsigExcC14NTransformToReferenceENS_4TextILm0EEE");
extern "C" void agiru_unlinked_922() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::AddXmlDsigExcC14NTransformToReference(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_923() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit37AddXmlDsigExcC14NTransformToReferenceEv");
extern "C" void agiru_unlinked_923() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::AddXmlDsigExcC14NTransformToReference()"); }
extern "C" void agiru_unlinked_924() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit50SetXmlDsigExcC14NTransformAsCanonicalizationMethodENS_4TextILm0EEE");
extern "C" void agiru_unlinked_924() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::SetXmlDsigExcC14NTransformAsCanonicalizationMethod(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_925() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit6GetXmlEv");
extern "C" void agiru_unlinked_925() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::GetXml()"); }
extern "C" void agiru_unlinked_926() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit7LoadXmlENS_10XmlElementE");
extern "C" void agiru_unlinked_926() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::LoadXml(agiru::XmlElement)"); }
extern "C" void agiru_unlinked_927() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit9AddClauseENS_10XmlElementE");
extern "C" void agiru_unlinked_927() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::AddClause(agiru::XmlElement)"); }
extern "C" void agiru_unlinked_928() asm("_ZN5agiru6System8Security10Encryption22SignedXmlImpl_Codeunit9AddObjectENS_10XmlElementE");
extern "C" void agiru_unlinked_928() { Unlinked("agiru::System::Security::Encryption::SignedXmlImpl_Codeunit::AddObject(agiru::XmlElement)"); }
extern "C" void agiru_unlinked_929() asm("_ZN5agiru6System8Security10Encryption25XmlDotNetConvert_Codeunit10FromDotNetENS_6dotnet11XmlDocumentERNS_11XmlDocumentE");
extern "C" void agiru_unlinked_929() { Unlinked("agiru::System::Security::Encryption::XmlDotNetConvert_Codeunit::FromDotNet(agiru::dotnet::XmlDocument, agiru::XmlDocument&)"); }
extern "C" void agiru_unlinked_930() asm("_ZN5agiru6System8Security10Encryption25XmlDotNetConvert_Codeunit10FromDotNetENS_6dotnet11XmlDocumentERNS_11XmlDocumentEb");
extern "C" void agiru_unlinked_930() { Unlinked("agiru::System::Security::Encryption::XmlDotNetConvert_Codeunit::FromDotNet(agiru::dotnet::XmlDocument, agiru::XmlDocument&, bool)"); }
extern "C" void agiru_unlinked_931() asm("_ZN5agiru6System8Security10Encryption25XmlDotNetConvert_Codeunit8ToDotNetENS_11XmlDocumentERNS_6dotnet11XmlDocumentEb");
extern "C" void agiru_unlinked_931() { Unlinked("agiru::System::Security::Encryption::XmlDotNetConvert_Codeunit::ToDotNet(agiru::XmlDocument, agiru::dotnet::XmlDocument&, bool)"); }
extern "C" void agiru_unlinked_932() asm("_ZN5agiru6System8Security10Encryption28DotNet_SecureString_Codeunit10AppendCharENS_4CharE");
extern "C" void agiru_unlinked_932() { Unlinked("agiru::System::Security::Encryption::DotNet_SecureString_Codeunit::AppendChar(agiru::Char)"); }
extern "C" void agiru_unlinked_933() asm("_ZN5agiru6System8Security10Encryption28DotNet_SecureString_Codeunit12SecureStringEv");
extern "C" void agiru_unlinked_933() { Unlinked("agiru::System::Security::Encryption::DotNet_SecureString_Codeunit::SecureString()"); }
extern "C" void agiru_unlinked_934() asm("_ZN5agiru6System8Security10Encryption28DotNet_SecureString_Codeunit15GetSecureStringERNS_6dotnet12SecureStringE");
extern "C" void agiru_unlinked_934() { Unlinked("agiru::System::Security::Encryption::DotNet_SecureString_Codeunit::GetSecureString(agiru::dotnet::SecureString&)"); }
extern "C" void agiru_unlinked_935() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit13HasPrivateKeyENS_4TextILm0EEENS_10SecretTextE");
extern "C" void agiru_unlinked_935() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::HasPrivateKey(agiru::Text<0ul>, agiru::SecretText)"); }
extern "C" void agiru_unlinked_936() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit17VerifyCertificateERNS_4TextILm0EEENS_10SecretTextENS_4EnumINS2_20X509ContentType_EnumEEE");
extern "C" void agiru_unlinked_936() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::VerifyCertificate(agiru::Text<0ul>&, agiru::SecretText, agiru::Enum<agiru::System::Security::Encryption::X509ContentType_Enum>)"); }
extern "C" void agiru_unlinked_937() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit20GetCertificateIssuerENS_4TextILm0EEENS_10SecretTextERS5_");
extern "C" void agiru_unlinked_937() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificateIssuer(agiru::Text<0ul>, agiru::SecretText, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_938() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit21GetCertificateSubjectENS_4TextILm0EEENS_10SecretTextERS5_");
extern "C" void agiru_unlinked_938() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificateSubject(agiru::Text<0ul>, agiru::SecretText, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_939() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit23GetCertificateNotBeforeENS_4TextILm0EEENS_10SecretTextERNS_8DateTimeE");
extern "C" void agiru_unlinked_939() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificateNotBefore(agiru::Text<0ul>, agiru::SecretText, agiru::DateTime&)"); }
extern "C" void agiru_unlinked_940() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit23GetCertificatePublicKeyENS_4TextILm0EEENS_10SecretTextE");
extern "C" void agiru_unlinked_940() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificatePublicKey(agiru::Text<0ul>, agiru::SecretText)"); }
extern "C" void agiru_unlinked_941() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit24GetCertificateExpirationENS_4TextILm0EEENS_10SecretTextERNS_8DateTimeE");
extern "C" void agiru_unlinked_941() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificateExpiration(agiru::Text<0ul>, agiru::SecretText, agiru::DateTime&)"); }
extern "C" void agiru_unlinked_942() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit24GetCertificateThumbprintENS_4TextILm0EEENS_10SecretTextERS5_");
extern "C" void agiru_unlinked_942() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificateThumbprint(agiru::Text<0ul>, agiru::SecretText, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_943() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit25InitializeX509CertificateENS_4TextILm0EEENS_10SecretTextERNS_6dotnet16X509Certificate2E");
extern "C" void agiru_unlinked_943() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::InitializeX509Certificate(agiru::Text<0ul>, agiru::SecretText, agiru::dotnet::X509Certificate2&)"); }
extern "C" void agiru_unlinked_944() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit26GetCertificateFriendlyNameENS_4TextILm0EEENS_10SecretTextERS5_");
extern "C" void agiru_unlinked_944() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificateFriendlyName(agiru::Text<0ul>, agiru::SecretText, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_945() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit26GetCertificateSerialNumberENS_4TextILm0EEENS_10SecretTextERS5_");
extern "C" void agiru_unlinked_945() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificateSerialNumber(agiru::Text<0ul>, agiru::SecretText, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_946() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit28GetRawCertDataAsBase64StringENS_4TextILm0EEENS_10SecretTextE");
extern "C" void agiru_unlinked_946() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetRawCertDataAsBase64String(agiru::Text<0ul>, agiru::SecretText)"); }
extern "C" void agiru_unlinked_947() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit30CreateFromPemAndExportAsBase64ENS_4TextILm0EEENS_10SecretTextES6_");
extern "C" void agiru_unlinked_947() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::CreateFromPemAndExportAsBase64(agiru::Text<0ul>, agiru::SecretText, agiru::SecretText)"); }
extern "C" void agiru_unlinked_948() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit30GetCertificatePropertiesAsJsonENS_4TextILm0EEENS_10SecretTextERS5_");
extern "C" void agiru_unlinked_948() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificatePropertiesAsJson(agiru::Text<0ul>, agiru::SecretText, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_949() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit30GetSecretCertificatePrivateKeyENS_4TextILm0EEENS_10SecretTextE");
extern "C" void agiru_unlinked_949() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetSecretCertificatePrivateKey(agiru::Text<0ul>, agiru::SecretText)"); }
extern "C" void agiru_unlinked_950() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit33GetCertificateSerialNumberAsASCIIENS_4TextILm0EEENS_10SecretTextERS5_");
extern "C" void agiru_unlinked_950() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificateSerialNumberAsASCII(agiru::Text<0ul>, agiru::SecretText, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_951() asm("_ZN5agiru6System8Security10Encryption29X509Certificate2Impl_Codeunit37GetCertificatePublicKeyAsBase64StringENS_4TextILm0EEENS_10SecretTextE");
extern "C" void agiru_unlinked_951() { Unlinked("agiru::System::Security::Encryption::X509Certificate2Impl_Codeunit::GetCertificatePublicKeyAsBase64String(agiru::Text<0ul>, agiru::SecretText)"); }
extern "C" void agiru_unlinked_952() asm("_ZN5agiru6System8Security10Encryption34IsolatedStorageManagement_Codeunit3GetENS_4TextILm0EEENS_9DataScopeERNS_10SecretTextE");
extern "C" void agiru_unlinked_952() { Unlinked("agiru::System::Security::Encryption::IsolatedStorageManagement_Codeunit::Get(agiru::Text<0ul>, agiru::DataScope, agiru::SecretText&)"); }
extern "C" void agiru_unlinked_953() asm("_ZN5agiru6System8Security10Encryption34IsolatedStorageManagement_Codeunit3SetENS_4TextILm0EEENS_10SecretTextENS_9DataScopeE");
extern "C" void agiru_unlinked_953() { Unlinked("agiru::System::Security::Encryption::IsolatedStorageManagement_Codeunit::Set(agiru::Text<0ul>, agiru::SecretText, agiru::DataScope)"); }
extern "C" void agiru_unlinked_954() asm("_ZN5agiru6System8Security10Encryption34IsolatedStorageManagement_Codeunit6DeleteENS_4TextILm0EEENS_9DataScopeE");
extern "C" void agiru_unlinked_954() { Unlinked("agiru::System::Security::Encryption::IsolatedStorageManagement_Codeunit::Delete(agiru::Text<0ul>, agiru::DataScope)"); }
extern "C" void agiru_unlinked_955() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_955() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_956() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page11OnNewRecordEb");
extern "C" void agiru_unlinked_956() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnNewRecord(bool)"); }
extern "C" void agiru_unlinked_957() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page14OnEditableNameEv");
extern "C" void agiru_unlinked_957() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnEditableName()"); }
extern "C" void agiru_unlinked_958() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page14OnInsertRecordEb");
extern "C" void agiru_unlinked_958() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnInsertRecord(bool)"); }
extern "C" void agiru_unlinked_959() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page14OnModifyRecordEv");
extern "C" void agiru_unlinked_959() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnModifyRecord()"); }
extern "C" void agiru_unlinked_960() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page14OnValidateNameEv");
extern "C" void agiru_unlinked_960() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnValidateName()"); }
extern "C" void agiru_unlinked_961() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_961() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_962() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page17OnActionWhereUsedEv");
extern "C" void agiru_unlinked_962() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnActionWhereUsed()"); }
extern "C" void agiru_unlinked_963() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_963() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_964() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page22OnActionSecurityGroupsEv");
extern "C" void agiru_unlinked_964() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnActionSecurityGroups()"); }
extern "C" void agiru_unlinked_965() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page23OnEditablePermissionSetEv");
extern "C" void agiru_unlinked_965() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnEditablePermissionSet()"); }
extern "C" void agiru_unlinked_966() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page23OnValidatePermissionSetEv");
extern "C" void agiru_unlinked_966() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnValidatePermissionSet()"); }
extern "C" void agiru_unlinked_967() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page24OnDrillDownPermissionSetEv");
extern "C" void agiru_unlinked_967() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnDrillDownPermissionSet()"); }
extern "C" void agiru_unlinked_968() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page25OnActionCopyPermissionSetEv");
extern "C" void agiru_unlinked_968() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnActionCopyPermissionSet()"); }
extern "C" void agiru_unlinked_969() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page26OnEnabledCopyPermissionSetEv");
extern "C" void agiru_unlinked_969() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnEnabledCopyPermissionSet()"); }
extern "C" void agiru_unlinked_970() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page27OnActionPermissionsOverviewEv");
extern "C" void agiru_unlinked_970() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnActionPermissionsOverview()"); }
extern "C" void agiru_unlinked_971() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page28OnActionExportPermissionSetsEv");
extern "C" void agiru_unlinked_971() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnActionExportPermissionSets()"); }
extern "C" void agiru_unlinked_972() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page28OnActionImportPermissionSetsEv");
extern "C" void agiru_unlinked_972() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnActionImportPermissionSets()"); }
extern "C" void agiru_unlinked_973() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page28OnActionPermissionSetContentEv");
extern "C" void agiru_unlinked_973() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnActionPermissionSetContent()"); }
extern "C" void agiru_unlinked_974() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page29OnEnabledImportPermissionSetsEv");
extern "C" void agiru_unlinked_974() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnEnabledImportPermissionSets()"); }
extern "C" void agiru_unlinked_975() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page31OnActionShowPermissionConflictsEv");
extern "C" void agiru_unlinked_975() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnActionShowPermissionConflicts()"); }
extern "C" void agiru_unlinked_976() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page32OnVisibleShowPermissionConflictsEv");
extern "C" void agiru_unlinked_976() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnVisibleShowPermissionConflicts()"); }
extern "C" void agiru_unlinked_977() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page33OnActionRemoveObsoletePermissionsEv");
extern "C" void agiru_unlinked_977() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnActionRemoveObsoletePermissions()"); }
extern "C" void agiru_unlinked_978() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page33OnVisiblePermissionSetAssignmentsEv");
extern "C" void agiru_unlinked_978() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnVisiblePermissionSetAssignments()"); }
extern "C" void agiru_unlinked_979() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page34OnEnabledRemoveObsoletePermissionsEv");
extern "C" void agiru_unlinked_979() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnEnabledRemoveObsoletePermissions()"); }
extern "C" void agiru_unlinked_980() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page39OnActionShowPermissionConflictsOverviewEv");
extern "C" void agiru_unlinked_980() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnActionShowPermissionConflictsOverview()"); }
extern "C" void agiru_unlinked_981() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page40OnVisibleShowPermissionConflictsOverviewEv");
extern "C" void agiru_unlinked_981() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnVisibleShowPermissionConflictsOverview()"); }
extern "C" void agiru_unlinked_982() asm("_ZN5agiru6System8Security13AccessControl19PermissionSets_Page6OnInitEv");
extern "C" void agiru_unlinked_982() { Unlinked("agiru::System::Security::AccessControl::PermissionSets_Page::OnInit()"); }
extern "C" void agiru_unlinked_983() asm("_ZN5agiru6System8Security13AccessControl22PermissionSetList_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_983() { Unlinked("agiru::System::Security::AccessControl::PermissionSetList_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_984() asm("_ZN5agiru6System8Security13AccessControl22PermissionSetList_Page18GetSelectionFilterERNS_6absent22AggregatePermissionSetE");
extern "C" void agiru_unlinked_984() { Unlinked("agiru::System::Security::AccessControl::PermissionSetList_Page::GetSelectionFilter(agiru::absent::AggregatePermissionSet&)"); }
extern "C" void agiru_unlinked_985() asm("_ZN5agiru6System8Security13AccessControl24PermissionsOverview_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_985() { Unlinked("agiru::System::Security::AccessControl::PermissionsOverview_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_986() asm("_ZN5agiru6System8Security13AccessControl24PermissionsOverview_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_986() { Unlinked("agiru::System::Security::AccessControl::PermissionsOverview_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_987() asm("_ZN5agiru6System8Security13AccessControl24PermissionsOverview_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_987() { Unlinked("agiru::System::Security::AccessControl::PermissionsOverview_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_988() asm("_ZN5agiru6System8Security13AccessControl24PermissionsOverview_Page22SetInitialObjectFilterENS_6OptionINS_7options64OptionNoneTableDataTableBlankReportBlank5CodeunitXMLpo_385374110EEENS_4TextILm0EEE");
extern "C" void agiru_unlinked_988() { Unlinked("agiru::System::Security::AccessControl::PermissionsOverview_Page::SetInitialObjectFilter(agiru::Option<agiru::options::OptionNoneTableDataTableBlankReportBlank5CodeunitXMLpo_385374110>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_989() asm("_ZN5agiru6System8Security13AccessControl24PermissionsOverview_Page22SetInitialRoleIDFilterENS_4TextILm30EEE");
extern "C" void agiru_unlinked_989() { Unlinked("agiru::System::Security::AccessControl::PermissionsOverview_Page::SetInitialRoleIDFilter(agiru::Text<30ul>)"); }
extern "C" void agiru_unlinked_990() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit10GetMembersERNS2_31SecurityGroupMemberBuffer_TableE");
extern "C" void agiru_unlinked_990() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::GetMembers(agiru::System::Security::AccessControl::SecurityGroupMemberBuffer_Table&)"); }
extern "C" void agiru_unlinked_991() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit11GetIdByNameENS_4TextILm0EEE");
extern "C" void agiru_unlinked_991() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::GetIdByName(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_992() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit14TryGetNameByIdENS_4TextILm0EEERS5_");
extern "C" void agiru_unlinked_992() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::TryGetNameById(agiru::Text<0ul>, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_993() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit15CopyPermissionsENS_4CodeILm20EEES5_");
extern "C" void agiru_unlinked_993() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::CopyPermissions(agiru::Code<20ul>, agiru::Code<20ul>)"); }
extern "C" void agiru_unlinked_994() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit15ValidateGroupIdENS_4TextILm0EEE");
extern "C" void agiru_unlinked_994() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::ValidateGroupId(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_995() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit16AddPermissionSetENS_4CodeILm20EEES5_NS_4TextILm30EEENS_6OptionINS_7options18OptionSystemTenantEEENS_4GuidE");
extern "C" void agiru_unlinked_995() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::AddPermissionSet(agiru::Code<20ul>, agiru::Code<20ul>, agiru::Text<30ul>, agiru::Option<agiru::options::OptionSystemTenant>, agiru::Guid)"); }
extern "C" void agiru_unlinked_996() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit16GetDesirableCodeENS_4TextILm0EEE");
extern "C" void agiru_unlinked_996() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::GetDesirableCode(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_997() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit18GetAvailableGroupsERNS2_25SecurityGroupBuffer_TableE");
extern "C" void agiru_unlinked_997() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::GetAvailableGroups(agiru::System::Security::AccessControl::SecurityGroupBuffer_Table&)"); }
extern "C" void agiru_unlinked_998() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit19RemovePermissionSetENS_4CodeILm20EEES5_NS_4TextILm30EEENS_6OptionINS_7options18OptionSystemTenantEEENS_4GuidE");
extern "C" void agiru_unlinked_998() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::RemovePermissionSet(agiru::Code<20ul>, agiru::Code<20ul>, agiru::Text<30ul>, agiru::Option<agiru::options::OptionSystemTenant>, agiru::Guid)"); }
extern "C" void agiru_unlinked_999() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit22GetGroupUserSecurityIdENS_4CodeILm20EEE");
extern "C" void agiru_unlinked_999() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::GetGroupUserSecurityId(agiru::Code<20ul>)"); }
extern "C" void agiru_unlinked_1000() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit23IsWindowsAuthenticationEv");
extern "C" void agiru_unlinked_1000() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::IsWindowsAuthentication()"); }
extern "C" void agiru_unlinked_1001() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit32SendNotificationForDeletedGroupsERNS2_25SecurityGroupBuffer_TableE");
extern "C" void agiru_unlinked_1001() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::SendNotificationForDeletedGroups(agiru::System::Security::AccessControl::SecurityGroupBuffer_Table&)"); }
extern "C" void agiru_unlinked_1002() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit4CopyENS_4CodeILm20EEES5_NS_4TextILm0EEE");
extern "C" void agiru_unlinked_1002() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::Copy(agiru::Code<20ul>, agiru::Code<20ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1003() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit5GetIdENS_4CodeILm20EEE");
extern "C" void agiru_unlinked_1003() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::GetId(agiru::Code<20ul>)"); }
extern "C" void agiru_unlinked_1004() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit6CreateENS_4CodeILm20EEENS_4TextILm0EEE");
extern "C" void agiru_unlinked_1004() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::Create(agiru::Code<20ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1005() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit6DeleteENS_4CodeILm20EEE");
extern "C" void agiru_unlinked_1005() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::Delete(agiru::Code<20ul>)"); }
extern "C" void agiru_unlinked_1006() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit6ExportENS_4ListINS_4CodeILm20EEEEENS_9OutStreamE");
extern "C" void agiru_unlinked_1006() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::Export(agiru::List<agiru::Code<20ul> >, agiru::OutStream)"); }
extern "C" void agiru_unlinked_1007() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit6ImportENS_8InStreamE");
extern "C" void agiru_unlinked_1007() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::Import(agiru::InStream)"); }
extern "C" void agiru_unlinked_1008() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit7GetCodeENS_4TextILm250EEERNS_4CodeILm0EEE");
extern "C" void agiru_unlinked_1008() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::GetCode(agiru::Text<250ul>, agiru::Code<0ul>&)"); }
extern "C" void agiru_unlinked_1009() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit7GetNameENS_4CodeILm20EEERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_1009() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::GetName(agiru::Code<20ul>, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_1010() asm("_ZN5agiru6System8Security13AccessControl26SecurityGroupImpl_Codeunit9GetGroupsERNS2_25SecurityGroupBuffer_TableEb");
extern "C" void agiru_unlinked_1010() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupImpl_Codeunit::GetGroups(agiru::System::Security::AccessControl::SecurityGroupBuffer_Table&, bool)"); }
extern "C" void agiru_unlinked_1011() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit19ConvertToPermissionENS_6OptionIvEE");
extern "C" void agiru_unlinked_1011() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::ConvertToPermission(agiru::Option<void>)"); }
extern "C" void agiru_unlinked_1012() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit19GetPermissionStatusENS_4EnumINS2_15Permission_EnumEEES6_b");
extern "C" void agiru_unlinked_1012() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::GetPermissionStatus(agiru::Enum<agiru::System::Security::AccessControl::Permission_Enum>, agiru::Enum<agiru::System::Security::AccessControl::Permission_Enum>, bool)"); }
extern "C" void agiru_unlinked_1013() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit22ShowPermissionConflictENS_4EnumINS2_15Permission_EnumEEES6_b");
extern "C" void agiru_unlinked_1013() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::ShowPermissionConflict(agiru::Enum<agiru::System::Security::AccessControl::Permission_Enum>, agiru::Enum<agiru::System::Security::AccessControl::Permission_Enum>, bool)"); }
extern "C" void agiru_unlinked_1014() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit23OpenPermissionConflictsENS_4CodeILm20EEENS_4EnumINS2_13Licenses_EnumEEE");
extern "C" void agiru_unlinked_1014() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::OpenPermissionConflicts(agiru::Code<20ul>, agiru::Enum<agiru::System::Security::AccessControl::Licenses_Enum>)"); }
extern "C" void agiru_unlinked_1015() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit24PopulatePermissionBufferERNS2_22PermissionBuffer_TableENS_4GuidENS_4TextILm50EEEii");
extern "C" void agiru_unlinked_1015() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::PopulatePermissionBuffer(agiru::System::Security::AccessControl::PermissionBuffer_Table&, agiru::Guid, agiru::Text<50ul>, int, int)"); }
extern "C" void agiru_unlinked_1016() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit32PopulatePermissionConflictsTableENS_4EnumINS2_13Licenses_EnumEEENS_4CodeILm20EEERNS2_25PermissionConflicts_TableE");
extern "C" void agiru_unlinked_1016() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::PopulatePermissionConflictsTable(agiru::Enum<agiru::System::Security::AccessControl::Licenses_Enum>, agiru::Code<20ul>, agiru::System::Security::AccessControl::PermissionConflicts_Table&)"); }
extern "C" void agiru_unlinked_1017() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit34HasDirectRIMPermissionsOnTableDataEi");
extern "C" void agiru_unlinked_1017() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::HasDirectRIMPermissionsOnTableData(int)"); }
extern "C" void agiru_unlinked_1018() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit34PopulateEffectivePermissionsBufferERNS_6absent10PermissionENS_4GuidENS_4TextILm50EEEiib");
extern "C" void agiru_unlinked_1018() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::PopulateEffectivePermissionsBuffer(agiru::absent::Permission&, agiru::Guid, agiru::Text<50ul>, int, int, bool)"); }
extern "C" void agiru_unlinked_1019() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit40PopulatePermissionConflictsOverviewTableERNS2_33PermissionConflictsOverview_TableENS_10DictionaryINS_4GuidEbEE");
extern "C" void agiru_unlinked_1019() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::PopulatePermissionConflictsOverviewTable(agiru::System::Security::AccessControl::PermissionConflictsOverview_Table&, agiru::Dictionary<agiru::Guid, bool>)"); }
extern "C" void agiru_unlinked_1020() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit51DisallowViewingEffectivePermissionsForNonAdminUsersENS_4GuidE");
extern "C" void agiru_unlinked_1020() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::DisallowViewingEffectivePermissionsForNonAdminUsers(agiru::Guid)"); }
extern "C" void agiru_unlinked_1021() asm("_ZN5agiru6System8Security13AccessControl32EffectivePermissionsMgt_Codeunit57PopulatePermissionRecordWithEffectivePermissionsForObjectERNS_6absent10PermissionENS_4GuidENS_4TextILm50EEENS_6OptionIvEEi");
extern "C" void agiru_unlinked_1021() { Unlinked("agiru::System::Security::AccessControl::EffectivePermissionsMgt_Codeunit::PopulatePermissionRecordWithEffectivePermissionsForObject(agiru::absent::Permission&, agiru::Guid, agiru::Text<50ul>, agiru::Option<void>, int)"); }
extern "C" void agiru_unlinked_1022() asm("_ZN5agiru6System8Security13AccessControl32SecurityGroupPermissionSets_Page12SetGroupCodeENS_4CodeILm20EEE");
extern "C" void agiru_unlinked_1022() { Unlinked("agiru::System::Security::AccessControl::SecurityGroupPermissionSets_Page::SetGroupCode(agiru::Code<20ul>)"); }
extern "C" void agiru_unlinked_1023() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit12SetStyleExprERNS2_33PermissionSetRelationBuffer_TableERNS_4TextILm0EEES8_");
extern "C" void agiru_unlinked_1023() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::SetStyleExpr(agiru::System::Security::AccessControl::PermissionSetRelationBuffer_Table&, agiru::Text<0ul>&, agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_1024() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit17GetPermissionSetsERNS2_25PermissionSetBuffer_TableE");
extern "C" void agiru_unlinked_1024() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::GetPermissionSets(agiru::System::Security::AccessControl::PermissionSetBuffer_Table&)"); }
extern "C" void agiru_unlinked_1025() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit19ModifyPermissionSetENS_4GuidENS_4CodeILm30EEENS_6OptionINS_7options18OptionSystemTenantEEES4_S6_NS7_INS8_20OptionIncludeExcludeEEE");
extern "C" void agiru_unlinked_1025() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::ModifyPermissionSet(agiru::Guid, agiru::Code<30ul>, agiru::Option<agiru::options::OptionSystemTenant>, agiru::Guid, agiru::Code<30ul>, agiru::Option<agiru::options::OptionIncludeExclude>)"); }
extern "C" void agiru_unlinked_1026() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit19RemovePermissionSetENS_4GuidENS_4CodeILm30EEES4_S6_");
extern "C" void agiru_unlinked_1026() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::RemovePermissionSet(agiru::Guid, agiru::Code<30ul>, agiru::Guid, agiru::Code<30ul>)"); }
extern "C" void agiru_unlinked_1027() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit20ExcludePermissionSetENS_4GuidENS_4CodeILm30EEENS_6OptionINS_7options18OptionSystemTenantEEES4_S6_SA_NS7_INS8_20OptionIncludeExcludeEEE");
extern "C" void agiru_unlinked_1027() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::ExcludePermissionSet(agiru::Guid, agiru::Code<30ul>, agiru::Option<agiru::options::OptionSystemTenant>, agiru::Guid, agiru::Code<30ul>, agiru::Option<agiru::options::OptionSystemTenant>, agiru::Option<agiru::options::OptionIncludeExclude>)"); }
extern "C" void agiru_unlinked_1028() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit20SelectPermissionSetsENS_4GuidENS_4CodeILm30EEENS_6OptionINS_7options18OptionSystemTenantEEE");
extern "C" void agiru_unlinked_1028() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::SelectPermissionSets(agiru::Guid, agiru::Code<30ul>, agiru::Option<agiru::options::OptionSystemTenant>)"); }
extern "C" void agiru_unlinked_1029() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit21OpenPermissionSetPageENS_4TextILm0EEENS_4CodeILm30EEENS_4GuidENS_6OptionINS_7options18OptionSystemTenantEEE");
extern "C" void agiru_unlinked_1029() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::OpenPermissionSetPage(agiru::Text<0ul>, agiru::Code<30ul>, agiru::Guid, agiru::Option<agiru::options::OptionSystemTenant>)"); }
extern "C" void agiru_unlinked_1030() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit21RefreshPermissionSetsENS_4CodeILm30EEENS_4GuidENS_6OptionIvEE");
extern "C" void agiru_unlinked_1030() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::RefreshPermissionSets(agiru::Code<30ul>, agiru::Guid, agiru::Option<void>)"); }
extern "C" void agiru_unlinked_1031() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit23ModifyPermissionSetTypeENS_4GuidENS_4CodeILm30EEENS_6OptionINS_7options18OptionSystemTenantEEES4_S6_NS7_INS8_20OptionIncludeExcludeEEE");
extern "C" void agiru_unlinked_1031() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::ModifyPermissionSetType(agiru::Guid, agiru::Code<30ul>, agiru::Option<agiru::options::OptionSystemTenant>, agiru::Guid, agiru::Code<30ul>, agiru::Option<agiru::options::OptionIncludeExclude>)"); }
extern "C" void agiru_unlinked_1032() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit27AddNewPermissionSetRelationENS_4GuidENS_4CodeILm30EEENS_6OptionINS_7options18OptionSystemTenantEEES4_S6_SA_NS7_INS8_20OptionIncludeExcludeEEE");
extern "C" void agiru_unlinked_1032() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::AddNewPermissionSetRelation(agiru::Guid, agiru::Code<30ul>, agiru::Option<agiru::options::OptionSystemTenant>, agiru::Guid, agiru::Code<30ul>, agiru::Option<agiru::options::OptionSystemTenant>, agiru::Option<agiru::options::OptionIncludeExclude>)"); }
extern "C" void agiru_unlinked_1033() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit28UpdateIncludedPermissionSetsENS_4TextILm0EEERNS2_25PermissionSetBuffer_TableE");
extern "C" void agiru_unlinked_1033() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::UpdateIncludedPermissionSets(agiru::Text<0ul>, agiru::System::Security::AccessControl::PermissionSetBuffer_Table&)"); }
extern "C" void agiru_unlinked_1034() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit30LookupAssignablePermissionSetsEbRNS_6absent22AggregatePermissionSetE");
extern "C" void agiru_unlinked_1034() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::LookupAssignablePermissionSets(bool, agiru::absent::AggregatePermissionSet&)"); }
extern "C" void agiru_unlinked_1035() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit30VerifyUserCanEditPermissionSetENS_4GuidE");
extern "C" void agiru_unlinked_1035() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::VerifyUserCanEditPermissionSet(agiru::Guid)"); }
extern "C" void agiru_unlinked_1036() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit34AddPermissionSetRelationBufferListERNS2_33PermissionSetRelationBuffer_TableE");
extern "C" void agiru_unlinked_1036() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::AddPermissionSetRelationBufferList(agiru::System::Security::AccessControl::PermissionSetRelationBuffer_Table&)"); }
extern "C" void agiru_unlinked_1037() asm("_ZN5agiru6System8Security13AccessControl34PermissionSetRelationImpl_Codeunit34AddPermissionSetRelationBufferTreeERNS2_33PermissionSetRelationBuffer_TableE");
extern "C" void agiru_unlinked_1037() { Unlinked("agiru::System::Security::AccessControl::PermissionSetRelationImpl_Codeunit::AddPermissionSetRelationBufferTree(agiru::System::Security::AccessControl::PermissionSetRelationBuffer_Table&)"); }
extern "C" void agiru_unlinked_1038() asm("_ZN5agiru6System8Security4User10Users_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_1038() { Unlinked("agiru::System::Security::User::Users_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_1039() asm("_ZN5agiru6System8Security4User10Users_Page11OnNewRecordEb");
extern "C" void agiru_unlinked_1039() { Unlinked("agiru::System::Security::User::Users_Page::OnNewRecord(bool)"); }
extern "C" void agiru_unlinked_1040() asm("_ZN5agiru6System8Security4User10Users_Page13OnActionEmailEv");
extern "C" void agiru_unlinked_1040() { Unlinked("agiru::System::Security::User::Users_Page::OnActionEmail()"); }
extern "C" void agiru_unlinked_1041() asm("_ZN5agiru6System8Security4User10Users_Page14OnEnabledEmailEv");
extern "C" void agiru_unlinked_1041() { Unlinked("agiru::System::Security::User::Users_Page::OnEnabledEmail()"); }
extern "C" void agiru_unlinked_1042() asm("_ZN5agiru6System8Security4User10Users_Page14OnInsertRecordEb");
extern "C" void agiru_unlinked_1042() { Unlinked("agiru::System::Security::User::Users_Page::OnInsertRecord(bool)"); }
extern "C" void agiru_unlinked_1043() asm("_ZN5agiru6System8Security4User10Users_Page14OnVisiblePlansEv");
extern "C" void agiru_unlinked_1043() { Unlinked("agiru::System::Security::User::Users_Page::OnVisiblePlans()"); }
extern "C" void agiru_unlinked_1044() asm("_ZN5agiru6System8Security4User10Users_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_1044() { Unlinked("agiru::System::Security::User::Users_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_1045() asm("_ZN5agiru6System8Security4User10Users_Page17OnActionEmployeesEv");
extern "C" void agiru_unlinked_1045() { Unlinked("agiru::System::Security::User::Users_Page::OnActionEmployees()"); }
extern "C" void agiru_unlinked_1046() asm("_ZN5agiru6System8Security4User10Users_Page17OnActionResourcesEv");
extern "C" void agiru_unlinked_1046() { Unlinked("agiru::System::Security::User::Users_Page::OnActionResources()"); }
extern "C" void agiru_unlinked_1047() asm("_ZN5agiru6System8Security4User10Users_Page17OnActionUserSetupEv");
extern "C" void agiru_unlinked_1047() { Unlinked("agiru::System::Security::User::Users_Page::OnActionUserSetup()"); }
extern "C" void agiru_unlinked_1048() asm("_ZN5agiru6System8Security4User10Users_Page18OnActionSentEmailsEv");
extern "C" void agiru_unlinked_1048() { Unlinked("agiru::System::Security::User::Users_Page::OnActionSentEmails()"); }
extern "C" void agiru_unlinked_1049() asm("_ZN5agiru6System8Security4User10Users_Page18OnEditableFullNameEv");
extern "C" void agiru_unlinked_1049() { Unlinked("agiru::System::Security::User::Users_Page::OnEditableFullName()"); }
extern "C" void agiru_unlinked_1050() asm("_ZN5agiru6System8Security4User10Users_Page18OnValidateUserNameEv");
extern "C" void agiru_unlinked_1050() { Unlinked("agiru::System::Security::User::Users_Page::OnValidateUserName()"); }
extern "C" void agiru_unlinked_1051() asm("_ZN5agiru6System8Security4User10Users_Page18OnVisibleControl18Ev");
extern "C" void agiru_unlinked_1051() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleControl18()"); }
extern "C" void agiru_unlinked_1052() asm("_ZN5agiru6System8Security4User10Users_Page19OnActionUserDetailsEv");
extern "C" void agiru_unlinked_1052() { Unlinked("agiru::System::Security::User::Users_Page::OnActionUserDetails()"); }
extern "C" void agiru_unlinked_1053() asm("_ZN5agiru6System8Security4User10Users_Page20OnActionAddMeAsSuperEv");
extern "C" void agiru_unlinked_1053() { Unlinked("agiru::System::Security::User::Users_Page::OnActionAddMeAsSuper()"); }
extern "C" void agiru_unlinked_1054() asm("_ZN5agiru6System8Security4User10Users_Page20OnActionUserSettingsEv");
extern "C" void agiru_unlinked_1054() { Unlinked("agiru::System::Security::User::Users_Page::OnActionUserSettings()"); }
extern "C" void agiru_unlinked_1055() asm("_ZN5agiru6System8Security4User10Users_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_1055() { Unlinked("agiru::System::Security::User::Users_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_1056() asm("_ZN5agiru6System8Security4User10Users_Page20OnVisibleLicenseTypeEv");
extern "C" void agiru_unlinked_1056() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleLicenseType()"); }
extern "C" void agiru_unlinked_1057() asm("_ZN5agiru6System8Security4User10Users_Page21OnVisibleAddMeAsSuperEv");
extern "C" void agiru_unlinked_1057() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleAddMeAsSuper()"); }
extern "C" void agiru_unlinked_1058() asm("_ZN5agiru6System8Security4User10Users_Page22OnActionFAJournalSetupEv");
extern "C" void agiru_unlinked_1058() { Unlinked("agiru::System::Security::User::Users_Page::OnActionFAJournalSetup()"); }
extern "C" void agiru_unlinked_1059() asm("_ZN5agiru6System8Security4User10Users_Page22OnActionPermissionSetsEv");
extern "C" void agiru_unlinked_1059() { Unlinked("agiru::System::Security::User::Users_Page::OnActionPermissionSets()"); }
extern "C" void agiru_unlinked_1060() asm("_ZN5agiru6System8Security4User10Users_Page22OnActionSecurityGroupsEv");
extern "C" void agiru_unlinked_1060() { Unlinked("agiru::System::Security::User::Users_Page::OnActionSecurityGroups()"); }
extern "C" void agiru_unlinked_1061() asm("_ZN5agiru6System8Security4User10Users_Page22OnActionUserTaskGroupsEv");
extern "C" void agiru_unlinked_1061() { Unlinked("agiru::System::Security::User::Users_Page::OnActionUserTaskGroups()"); }
extern "C" void agiru_unlinked_1062() asm("_ZN5agiru6System8Security4User10Users_Page24OnVisibleWindowsUserNameEv");
extern "C" void agiru_unlinked_1062() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleWindowsUserName()"); }
extern "C" void agiru_unlinked_1063() asm("_ZN5agiru6System8Security4User10Users_Page25OnActionPrinterSelectionsEv");
extern "C" void agiru_unlinked_1063() { Unlinked("agiru::System::Security::User::Users_Page::OnActionPrinterSelections()"); }
extern "C" void agiru_unlinked_1064() asm("_ZN5agiru6System8Security4User10Users_Page25OnActionUserEmailPoliciesEv");
extern "C" void agiru_unlinked_1064() { Unlinked("agiru::System::Security::User::Users_Page::OnActionUserEmailPolicies()"); }
extern "C" void agiru_unlinked_1065() asm("_ZN5agiru6System8Security4User10Users_Page25OnValidateWindowsUserNameEv");
extern "C" void agiru_unlinked_1065() { Unlinked("agiru::System::Security::User::Users_Page::OnValidateWindowsUserName()"); }
extern "C" void agiru_unlinked_1066() asm("_ZN5agiru6System8Security4User10Users_Page26OnActionWarehouseEmployeesEv");
extern "C" void agiru_unlinked_1066() { Unlinked("agiru::System::Security::User::Users_Page::OnActionWarehouseEmployees()"); }
extern "C" void agiru_unlinked_1067() asm("_ZN5agiru6System8Security4User10Users_Page27OnEditableWindowsSecurityIDEv");
extern "C" void agiru_unlinked_1067() { Unlinked("agiru::System::Security::User::Users_Page::OnEditableWindowsSecurityID()"); }
extern "C" void agiru_unlinked_1068() asm("_ZN5agiru6System8Security4User10Users_Page27OnVisibleUserSecurityGroupsEv");
extern "C" void agiru_unlinked_1068() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleUserSecurityGroups()"); }
extern "C" void agiru_unlinked_1069() asm("_ZN5agiru6System8Security4User10Users_Page28OnActionEffectivePermissionsEv");
extern "C" void agiru_unlinked_1069() { Unlinked("agiru::System::Security::User::Users_Page::OnActionEffectivePermissions()"); }
extern "C" void agiru_unlinked_1070() asm("_ZN5agiru6System8Security4User10Users_Page28OnVisibleAuthenticationEmailEv");
extern "C" void agiru_unlinked_1070() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleAuthenticationEmail()"); }
extern "C" void agiru_unlinked_1071() asm("_ZN5agiru6System8Security4User10Users_Page29OnActionUpdateUsersFromOfficeEv");
extern "C" void agiru_unlinked_1071() { Unlinked("agiru::System::Security::User::Users_Page::OnActionUpdateUsersFromOffice()"); }
extern "C" void agiru_unlinked_1072() asm("_ZN5agiru6System8Security4User10Users_Page30OnActionSalespersonsPurchasersEv");
extern "C" void agiru_unlinked_1072() { Unlinked("agiru::System::Security::User::Users_Page::OnActionSalespersonsPurchasers()"); }
extern "C" void agiru_unlinked_1073() asm("_ZN5agiru6System8Security4User10Users_Page30OnEnabledUpdateUsersFromOfficeEv");
extern "C" void agiru_unlinked_1073() { Unlinked("agiru::System::Security::User::Users_Page::OnEnabledUpdateUsersFromOffice()"); }
extern "C" void agiru_unlinked_1074() asm("_ZN5agiru6System8Security4User10Users_Page30OnVisibleUpdateUsersFromOfficeEv");
extern "C" void agiru_unlinked_1074() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleUpdateUsersFromOffice()"); }
extern "C" void agiru_unlinked_1075() asm("_ZN5agiru6System8Security4User10Users_Page32OnActionInviteExternalAccountantEv");
extern "C" void agiru_unlinked_1075() { Unlinked("agiru::System::Security::User::Users_Page::OnActionInviteExternalAccountant()"); }
extern "C" void agiru_unlinked_1076() asm("_ZN5agiru6System8Security4User10Users_Page32OnVisibleInheritedPermissionSetsEv");
extern "C" void agiru_unlinked_1076() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleInheritedPermissionSets()"); }
extern "C" void agiru_unlinked_1077() asm("_ZN5agiru6System8Security4User10Users_Page33OnActionTriggerPageBackgroundTaskEv");
extern "C" void agiru_unlinked_1077() { Unlinked("agiru::System::Security::User::Users_Page::OnActionTriggerPageBackgroundTask()"); }
extern "C" void agiru_unlinked_1078() asm("_ZN5agiru6System8Security4User10Users_Page33OnVisibleInviteExternalAccountantEv");
extern "C" void agiru_unlinked_1078() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleInviteExternalAccountant()"); }
extern "C" void agiru_unlinked_1079() asm("_ZN5agiru6System8Security4User10Users_Page34OnVisibleUserSecurityGroupsLoadingEv");
extern "C" void agiru_unlinked_1079() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleUserSecurityGroupsLoading()"); }
extern "C" void agiru_unlinked_1080() asm("_ZN5agiru6System8Security4User10Users_Page37OnActionRestoreUserDefaultPermissionsEv");
extern "C" void agiru_unlinked_1080() { Unlinked("agiru::System::Security::User::Users_Page::OnActionRestoreUserDefaultPermissions()"); }
extern "C" void agiru_unlinked_1081() asm("_ZN5agiru6System8Security4User10Users_Page38OnEnabledRestoreUserDefaultPermissionsEv");
extern "C" void agiru_unlinked_1081() { Unlinked("agiru::System::Security::User::Users_Page::OnEnabledRestoreUserDefaultPermissions()"); }
extern "C" void agiru_unlinked_1082() asm("_ZN5agiru6System8Security4User10Users_Page38OnVisibleRestoreUserDefaultPermissionsEv");
extern "C" void agiru_unlinked_1082() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleRestoreUserDefaultPermissions()"); }
extern "C" void agiru_unlinked_1083() asm("_ZN5agiru6System8Security4User10Users_Page39OnVisibleInheritedPermissionSetsLoadingEv");
extern "C" void agiru_unlinked_1083() { Unlinked("agiru::System::Security::User::Users_Page::OnVisibleInheritedPermissionSetsLoading()"); }
extern "C" void agiru_unlinked_1084() asm("_ZN5agiru6System8Security4User10Users_Page6OnInitEv");
extern "C" void agiru_unlinked_1084() { Unlinked("agiru::System::Security::User::Users_Page::OnInit()"); }
extern "C" void agiru_unlinked_1085() asm("_ZN5agiru6System8Security4User13UserCard_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_1085() { Unlinked("agiru::System::Security::User::UserCard_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_1086() asm("_ZN5agiru6System8Security4User13UserCard_Page11OnNewRecordEb");
extern "C" void agiru_unlinked_1086() { Unlinked("agiru::System::Security::User::UserCard_Page::OnNewRecord(bool)"); }
extern "C" void agiru_unlinked_1087() asm("_ZN5agiru6System8Security4User13UserCard_Page13OnActionEmailEv");
extern "C" void agiru_unlinked_1087() { Unlinked("agiru::System::Security::User::UserCard_Page::OnActionEmail()"); }
extern "C" void agiru_unlinked_1088() asm("_ZN5agiru6System8Security4User13UserCard_Page14OnInsertRecordEb");
extern "C" void agiru_unlinked_1088() { Unlinked("agiru::System::Security::User::UserCard_Page::OnInsertRecord(bool)"); }
extern "C" void agiru_unlinked_1089() asm("_ZN5agiru6System8Security4User13UserCard_Page14OnVisiblePlansEv");
extern "C" void agiru_unlinked_1089() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisiblePlans()"); }
extern "C" void agiru_unlinked_1090() asm("_ZN5agiru6System8Security4User13UserCard_Page15OnValidateStateEv");
extern "C" void agiru_unlinked_1090() { Unlinked("agiru::System::Security::User::UserCard_Page::OnValidateState()"); }
extern "C" void agiru_unlinked_1091() asm("_ZN5agiru6System8Security4User13UserCard_Page16OnActionAcsSetupEv");
extern "C" void agiru_unlinked_1091() { Unlinked("agiru::System::Security::User::UserCard_Page::OnActionAcsSetup()"); }
extern "C" void agiru_unlinked_1092() asm("_ZN5agiru6System8Security4User13UserCard_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_1092() { Unlinked("agiru::System::Security::User::UserCard_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_1093() asm("_ZN5agiru6System8Security4User13UserCard_Page16OnQueryClosePageENS_6ActionE");
extern "C" void agiru_unlinked_1093() { Unlinked("agiru::System::Security::User::UserCard_Page::OnQueryClosePage(agiru::Action)"); }
extern "C" void agiru_unlinked_1094() asm("_ZN5agiru6System8Security4User13UserCard_Page17OnVisibleAcsSetupEv");
extern "C" void agiru_unlinked_1094() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleAcsSetup()"); }
extern "C" void agiru_unlinked_1095() asm("_ZN5agiru6System8Security4User13UserCard_Page18OnActionSentEmailsEv");
extern "C" void agiru_unlinked_1095() { Unlinked("agiru::System::Security::User::UserCard_Page::OnActionSentEmails()"); }
extern "C" void agiru_unlinked_1096() asm("_ZN5agiru6System8Security4User13UserCard_Page18OnEditableFullNameEv");
extern "C" void agiru_unlinked_1096() { Unlinked("agiru::System::Security::User::UserCard_Page::OnEditableFullName()"); }
extern "C" void agiru_unlinked_1097() asm("_ZN5agiru6System8Security4User13UserCard_Page18OnValidateUserNameEv");
extern "C" void agiru_unlinked_1097() { Unlinked("agiru::System::Security::User::UserCard_Page::OnValidateUserName()"); }
extern "C" void agiru_unlinked_1098() asm("_ZN5agiru6System8Security4User13UserCard_Page18OnVisibleACSStatusEv");
extern "C" void agiru_unlinked_1098() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleACSStatus()"); }
extern "C" void agiru_unlinked_1099() asm("_ZN5agiru6System8Security4User13UserCard_Page19OnVisibleExpiryDateEv");
extern "C" void agiru_unlinked_1099() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleExpiryDate()"); }
extern "C" void agiru_unlinked_1100() asm("_ZN5agiru6System8Security4User13UserCard_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_1100() { Unlinked("agiru::System::Security::User::UserCard_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_1101() asm("_ZN5agiru6System8Security4User13UserCard_Page20OnAssistEditPasswordEv");
extern "C" void agiru_unlinked_1101() { Unlinked("agiru::System::Security::User::UserCard_Page::OnAssistEditPassword()"); }
extern "C" void agiru_unlinked_1102() asm("_ZN5agiru6System8Security4User13UserCard_Page20OnVisibleLicenseTypeEv");
extern "C" void agiru_unlinked_1102() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleLicenseType()"); }
extern "C" void agiru_unlinked_1103() asm("_ZN5agiru6System8Security4User13UserCard_Page20OnVisiblePermissionsEv");
extern "C" void agiru_unlinked_1103() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisiblePermissions()"); }
extern "C" void agiru_unlinked_1104() asm("_ZN5agiru6System8Security4User13UserCard_Page21OnAssistEditACSStatusEv");
extern "C" void agiru_unlinked_1104() { Unlinked("agiru::System::Security::User::UserCard_Page::OnAssistEditACSStatus()"); }
extern "C" void agiru_unlinked_1105() asm("_ZN5agiru6System8Security4User13UserCard_Page21OnValidateLicenseTypeEv");
extern "C" void agiru_unlinked_1105() { Unlinked("agiru::System::Security::User::UserCard_Page::OnValidateLicenseType()"); }
extern "C" void agiru_unlinked_1106() asm("_ZN5agiru6System8Security4User13UserCard_Page22OnVisibleApplicationIDEv");
extern "C" void agiru_unlinked_1106() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleApplicationID()"); }
extern "C" void agiru_unlinked_1107() asm("_ZN5agiru6System8Security4User13UserCard_Page23OnValidateApplicationIDEv");
extern "C" void agiru_unlinked_1107() { Unlinked("agiru::System::Security::User::UserCard_Page::OnValidateApplicationID()"); }
extern "C" void agiru_unlinked_1108() asm("_ZN5agiru6System8Security4User13UserCard_Page24OnActionChangePassword_2Ev");
extern "C" void agiru_unlinked_1108() { Unlinked("agiru::System::Security::User::UserCard_Page::OnActionChangePassword_2()"); }
extern "C" void agiru_unlinked_1109() asm("_ZN5agiru6System8Security4User13UserCard_Page24OnAssistEditWebServiceIDEv");
extern "C" void agiru_unlinked_1109() { Unlinked("agiru::System::Security::User::UserCard_Page::OnAssistEditWebServiceID()"); }
extern "C" void agiru_unlinked_1110() asm("_ZN5agiru6System8Security4User13UserCard_Page25OnActionRemoveWSAccessKeyEv");
extern "C" void agiru_unlinked_1110() { Unlinked("agiru::System::Security::User::UserCard_Page::OnActionRemoveWSAccessKey()"); }
extern "C" void agiru_unlinked_1111() asm("_ZN5agiru6System8Security4User13UserCard_Page25OnValidateWindowsUserNameEv");
extern "C" void agiru_unlinked_1111() { Unlinked("agiru::System::Security::User::UserCard_Page::OnValidateWindowsUserName()"); }
extern "C" void agiru_unlinked_1112() asm("_ZN5agiru6System8Security4User13UserCard_Page25OnVisibleWebServiceAccessEv");
extern "C" void agiru_unlinked_1112() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleWebServiceAccess()"); }
extern "C" void agiru_unlinked_1113() asm("_ZN5agiru6System8Security4User13UserCard_Page25RemoveWebServiceAccessKeyENS_4GuidE");
extern "C" void agiru_unlinked_1113() { Unlinked("agiru::System::Security::User::UserCard_Page::RemoveWebServiceAccessKey(agiru::Guid)"); }
extern "C" void agiru_unlinked_1114() asm("_ZN5agiru6System8Security4User13UserCard_Page26OnEnabledRemoveWSAccessKeyEv");
extern "C" void agiru_unlinked_1114() { Unlinked("agiru::System::Security::User::UserCard_Page::OnEnabledRemoveWSAccessKey()"); }
extern "C" void agiru_unlinked_1115() asm("_ZN5agiru6System8Security4User13UserCard_Page26OnVisibleACSAuthenticationEv");
extern "C" void agiru_unlinked_1115() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleACSAuthentication()"); }
extern "C" void agiru_unlinked_1116() asm("_ZN5agiru6System8Security4User13UserCard_Page27OnAssistEditUserTelemetryIDEv");
extern "C" void agiru_unlinked_1116() { Unlinked("agiru::System::Security::User::UserCard_Page::OnAssistEditUserTelemetryID()"); }
extern "C" void agiru_unlinked_1117() asm("_ZN5agiru6System8Security4User13UserCard_Page27OnEditableWindowsSecurityIDEv");
extern "C" void agiru_unlinked_1117() { Unlinked("agiru::System::Security::User::UserCard_Page::OnEditableWindowsSecurityID()"); }
extern "C" void agiru_unlinked_1118() asm("_ZN5agiru6System8Security4User13UserCard_Page28OnActionEffectivePermissionsEv");
extern "C" void agiru_unlinked_1118() { Unlinked("agiru::System::Security::User::UserCard_Page::OnActionEffectivePermissions()"); }
extern "C" void agiru_unlinked_1119() asm("_ZN5agiru6System8Security4User13UserCard_Page29OnEditableAuthenticationEmailEv");
extern "C" void agiru_unlinked_1119() { Unlinked("agiru::System::Security::User::UserCard_Page::OnEditableAuthenticationEmail()"); }
extern "C" void agiru_unlinked_1120() asm("_ZN5agiru6System8Security4User13UserCard_Page29OnValidateAuthenticationEmailEv");
extern "C" void agiru_unlinked_1120() { Unlinked("agiru::System::Security::User::UserCard_Page::OnValidateAuthenticationEmail()"); }
extern "C" void agiru_unlinked_1121() asm("_ZN5agiru6System8Security4User13UserCard_Page30OnVisibleWindowsAuthenticationEv");
extern "C" void agiru_unlinked_1121() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleWindowsAuthentication()"); }
extern "C" void agiru_unlinked_1122() asm("_ZN5agiru6System8Security4User13UserCard_Page32OnActionDeleteExchangeIdentifierEv");
extern "C" void agiru_unlinked_1122() { Unlinked("agiru::System::Security::User::UserCard_Page::OnActionDeleteExchangeIdentifier()"); }
extern "C" void agiru_unlinked_1123() asm("_ZN5agiru6System8Security4User13UserCard_Page33OnActionChangeWebServiceAccessKeyEv");
extern "C" void agiru_unlinked_1123() { Unlinked("agiru::System::Security::User::UserCard_Page::OnActionChangeWebServiceAccessKey()"); }
extern "C" void agiru_unlinked_1124() asm("_ZN5agiru6System8Security4User13UserCard_Page33OnEnabledDeleteExchangeIdentifierEv");
extern "C" void agiru_unlinked_1124() { Unlinked("agiru::System::Security::User::UserCard_Page::OnEnabledDeleteExchangeIdentifier()"); }
extern "C" void agiru_unlinked_1125() asm("_ZN5agiru6System8Security4User13UserCard_Page34OnEnabledChangeWebServiceAccessKeyEv");
extern "C" void agiru_unlinked_1125() { Unlinked("agiru::System::Security::User::UserCard_Page::OnEnabledChangeWebServiceAccessKey()"); }
extern "C" void agiru_unlinked_1126() asm("_ZN5agiru6System8Security4User13UserCard_Page34OnVisibleNAVPasswordAuthenticationEv");
extern "C" void agiru_unlinked_1126() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleNAVPasswordAuthentication()"); }
extern "C" void agiru_unlinked_1127() asm("_ZN5agiru6System8Security4User13UserCard_Page34OnVisibleUserSecurityGroupsLoadingEv");
extern "C" void agiru_unlinked_1127() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleUserSecurityGroupsLoading()"); }
extern "C" void agiru_unlinked_1128() asm("_ZN5agiru6System8Security4User13UserCard_Page39OnVisibleInheritedPermissionSetsLoadingEv");
extern "C" void agiru_unlinked_1128() { Unlinked("agiru::System::Security::User::UserCard_Page::OnVisibleInheritedPermissionSetsLoading()"); }
extern "C" void agiru_unlinked_1129() asm("_ZN5agiru6System8Security4User13UserCard_Page6OnInitEv");
extern "C" void agiru_unlinked_1129() { Unlinked("agiru::System::Security::User::UserCard_Page::OnInit()"); }
extern "C" void agiru_unlinked_1130() asm("_ZN5agiru6System9Telemetry31AuditLogImplementation_Codeunit15LogAuditMessageENS_4TextILm0EEENS_23SecurityOperationResultENS_13AuditCategoryEiiNS_10DictionaryIS4_S4_EENS_10ModuleInfoE");
extern "C" void agiru_unlinked_1130() { Unlinked("agiru::System::Telemetry::AuditLogImplementation_Codeunit::LogAuditMessage(agiru::Text<0ul>, agiru::SecurityOperationResult, agiru::AuditCategory, int, int, agiru::Dictionary<agiru::Text<0ul>, agiru::Text<0ul> >, agiru::ModuleInfo)"); }
extern "C" void agiru_unlinked_1131() asm("_ZN5agiru6System9Telemetry31AuditLogImplementation_Codeunit15LogAuditMessageENS_4TextILm0EEENS_23SecurityOperationResultENS_13AuditCategoryEiiNS_10ModuleInfoE");
extern "C" void agiru_unlinked_1131() { Unlinked("agiru::System::Telemetry::AuditLogImplementation_Codeunit::LogAuditMessage(agiru::Text<0ul>, agiru::SecurityOperationResult, agiru::AuditCategory, int, int, agiru::ModuleInfo)"); }
extern "C" void agiru_unlinked_1132() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit11RunNextTestERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1132() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::RunNextTest(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1133() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit14DeleteChildrenERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1133() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::DeleteChildren(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1134() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit15CalcTestResultsENS2_20TestMethodLine_TableERiS5_S5_S5_");
extern "C" void agiru_unlinked_1134() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::CalcTestResults(agiru::System::TestTools::TestRunner::TestMethodLine_Table, int&, int&, int&, int&)"); }
extern "C" void agiru_unlinked_1135() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit15CreateTestSuiteERNS_4CodeILm0EEE");
extern "C" void agiru_unlinked_1135() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::CreateTestSuite(agiru::Code<0ul>&)"); }
extern "C" void agiru_unlinked_1136() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit16ChangeTestRunnerERNS2_17ALTestSuite_TableEi");
extern "C" void agiru_unlinked_1136() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::ChangeTestRunner(agiru::System::TestTools::TestRunner::ALTestSuite_Table&, int)"); }
extern "C" void agiru_unlinked_1137() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit16ClearErrorOnLineERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1137() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::ClearErrorOnLine(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1138() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit16DeleteAllMethodsERNS2_17ALTestSuite_TableE");
extern "C" void agiru_unlinked_1138() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::DeleteAllMethods(agiru::System::TestTools::TestRunner::ALTestSuite_Table&)"); }
extern "C" void agiru_unlinked_1139() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit16IsTestMethodLineENS_4TextILm128EEE");
extern "C" void agiru_unlinked_1139() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::IsTestMethodLine(agiru::Text<128ul>)"); }
extern "C" void agiru_unlinked_1140() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit16LookupTestRunnerERNS2_17ALTestSuite_TableE");
extern "C" void agiru_unlinked_1140() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::LookupTestRunner(agiru::System::TestTools::TestRunner::ALTestSuite_Table&)"); }
extern "C" void agiru_unlinked_1141() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit16RunSelectedTestsERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1141() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::RunSelectedTests(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1142() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit17GetErrorCallStackERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1142() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::GetErrorCallStack(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1143() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit17GetLastTestLineNoENS2_17ALTestSuite_TableE");
extern "C" void agiru_unlinked_1143() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::GetLastTestLineNo(agiru::System::TestTools::TestRunner::ALTestSuite_Table)"); }
extern "C" void agiru_unlinked_1144() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit17SelectTestMethodsERNS2_17ALTestSuite_TableE");
extern "C" void agiru_unlinked_1144() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SelectTestMethods(agiru::System::TestTools::TestRunner::ALTestSuite_Table&)"); }
extern "C" void agiru_unlinked_1145() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit17SetCCTrackingTypeERNS2_17ALTestSuite_TableEi");
extern "C" void agiru_unlinked_1145() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SetCCTrackingType(agiru::System::TestTools::TestRunner::ALTestSuite_Table&, int)"); }
extern "C" void agiru_unlinked_1146() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit17TestResultsToJSONERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1146() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::TestResultsToJSON(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1147() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit17UpdateTestMethodsERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1147() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::UpdateTestMethods(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1148() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit18ChangeStabilityRunERNS2_17ALTestSuite_TableEb");
extern "C" void agiru_unlinked_1148() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::ChangeStabilityRun(agiru::System::TestTools::TestRunner::ALTestSuite_Table&, bool)"); }
extern "C" void agiru_unlinked_1149() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit18SetLastErrorOnLineERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1149() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SetLastErrorOnLine(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1150() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit19GetFullErrorMessageERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1150() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::GetFullErrorMessage(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1151() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit19GetNextMethodNumberENS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1151() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::GetNextMethodNumber(agiru::System::TestTools::TestRunner::TestMethodLine_Table)"); }
extern "C" void agiru_unlinked_1152() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit21RunTestSuiteSelectionERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1152() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::RunTestSuiteSelection(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1153() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit21SetCCTrackAllSessionsERNS2_17ALTestSuite_TableEb");
extern "C" void agiru_unlinked_1153() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SetCCTrackAllSessions(agiru::System::TestTools::TestRunner::ALTestSuite_Table&, bool)"); }
extern "C" void agiru_unlinked_1154() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit21ValidateTestMethodRunERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1154() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::ValidateTestMethodRun(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1155() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit22ValidateTestMethodNameERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1155() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::ValidateTestMethodName(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1156() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit24GetTestRunnerDisplayNameENS2_17ALTestSuite_TableE");
extern "C" void agiru_unlinked_1156() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::GetTestRunnerDisplayName(agiru::System::TestTools::TestRunner::ALTestSuite_Table)"); }
extern "C" void agiru_unlinked_1157() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit24LookupTestMethodsByRangeERNS2_17ALTestSuite_TableE");
extern "C" void agiru_unlinked_1157() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::LookupTestMethodsByRange(agiru::System::TestTools::TestRunner::ALTestSuite_Table&)"); }
extern "C" void agiru_unlinked_1158() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit24SelectTestMethodsByRangeERNS2_17ALTestSuite_TableENS_4TextILm0EEE");
extern "C" void agiru_unlinked_1158() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SelectTestMethodsByRange(agiru::System::TestTools::TestRunner::ALTestSuite_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1159() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit24SelectTestMethodsByRangeERNS2_17ALTestSuite_TableENS_4TextILm0EEEii");
extern "C" void agiru_unlinked_1159() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SelectTestMethodsByRange(agiru::System::TestTools::TestRunner::ALTestSuite_Table&, agiru::Text<0ul>, int, int)"); }
extern "C" void agiru_unlinked_1160() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit25SetCodeCoverageExporterIDERNS2_17ALTestSuite_TableEi");
extern "C" void agiru_unlinked_1160() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SetCodeCoverageExporterID(agiru::System::TestTools::TestRunner::ALTestSuite_Table&, int)"); }
extern "C" void agiru_unlinked_1161() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit26SelectTestProceduresByNameENS_4CodeILm10EEENS_4TextILm0EEE");
extern "C" void agiru_unlinked_1161() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SelectTestProceduresByName(agiru::Code<10ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1162() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit26ValidateTestMethodFunctionERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1162() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::ValidateTestMethodFunction(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1163() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit26ValidateTestMethodLineTypeERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1163() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::ValidateTestMethodLineType(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1164() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit28SelectTestMethodsByExtensionERNS2_17ALTestSuite_TableENS_4TextILm0EEE");
extern "C" void agiru_unlinked_1164() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SelectTestMethodsByExtension(agiru::System::TestTools::TestRunner::ALTestSuite_Table&, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1165() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit29GetErrorMessageWithStackTraceERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1165() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::GetErrorMessageWithStackTrace(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1166() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit30GetLineNoFilterForTestCodeunitENS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1166() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::GetLineNoFilterForTestCodeunit(agiru::System::TestTools::TestRunner::TestMethodLine_Table)"); }
extern "C" void agiru_unlinked_1167() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit30UpdateCodeCoverageTrackingTypeERNS2_17ALTestSuite_TableE");
extern "C" void agiru_unlinked_1167() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::UpdateCodeCoverageTrackingType(agiru::System::TestTools::TestRunner::ALTestSuite_Table&)"); }
extern "C" void agiru_unlinked_1168() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit30ValidateTestMethodTestCodeunitERNS2_20TestMethodLine_TableE");
extern "C" void agiru_unlinked_1168() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::ValidateTestMethodTestCodeunit(agiru::System::TestTools::TestRunner::TestMethodLine_Table&)"); }
extern "C" void agiru_unlinked_1169() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit35UpdateCodeCoverageTrackAllSesssionsERNS2_17ALTestSuite_TableE");
extern "C" void agiru_unlinked_1169() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::UpdateCodeCoverageTrackAllSesssions(agiru::System::TestTools::TestRunner::ALTestSuite_Table&)"); }
extern "C" void agiru_unlinked_1170() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit49SelectTestMethodsByExtensionAndTestCategorizationERNS2_17ALTestSuite_TableENS_4TextILm0EEEii");
extern "C" void agiru_unlinked_1170() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SelectTestMethodsByExtensionAndTestCategorization(agiru::System::TestTools::TestRunner::ALTestSuite_Table&, agiru::Text<0ul>, int, int)"); }
extern "C" void agiru_unlinked_1171() asm("_ZN5agiru6System9TestTools10TestRunner21TestSuiteMgt_Codeunit8SetCCMapERNS2_17ALTestSuite_TableEi");
extern "C" void agiru_unlinked_1171() { Unlinked("agiru::System::TestTools::TestRunner::TestSuiteMgt_Codeunit::SetCCMap(agiru::System::TestTools::TestRunner::ALTestSuite_Table&, int)"); }
extern "C" void agiru_unlinked_1172() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit10ISTESTMODEEv");
extern "C" void agiru_unlinked_1172() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::ISTESTMODE()"); }
extern "C" void agiru_unlinked_1173() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit11RunSelectedERNS2_17CALTestLine_TableE");
extern "C" void agiru_unlinked_1173() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::RunSelected(agiru::System::TestTools::TestRunner::CALTestLine_Table&)"); }
extern "C" void agiru_unlinked_1174() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit13ISPUBLISHMODEEv");
extern "C" void agiru_unlinked_1174() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::ISPUBLISHMODE()"); }
extern "C" void agiru_unlinked_1175() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit13RunSuiteYesNoERNS2_17CALTestLine_TableE");
extern "C" void agiru_unlinked_1175() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::RunSuiteYesNo(agiru::System::TestTools::TestRunner::CALTestLine_Table&)"); }
extern "C" void agiru_unlinked_1176() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit14CreateNewSuiteERNS_4CodeILm0EEE");
extern "C" void agiru_unlinked_1176() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::CreateNewSuite(agiru::Code<0ul>&)"); }
extern "C" void agiru_unlinked_1177() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit14SETPUBLISHMODEEv");
extern "C" void agiru_unlinked_1177() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::SETPUBLISHMODE()"); }
extern "C" void agiru_unlinked_1178() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit16AddTestCodeunitsENS2_18CALTestSuite_TableERNS_8platform23AllObjWithCaption_TableE");
extern "C" void agiru_unlinked_1178() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::AddTestCodeunits(agiru::System::TestTools::TestRunner::CALTestSuite_Table, agiru::platform::AllObjWithCaption_Table&)"); }
extern "C" void agiru_unlinked_1179() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit18ExtendTestCoverageEi");
extern "C" void agiru_unlinked_1179() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::ExtendTestCoverage(int)"); }
extern "C" void agiru_unlinked_1180() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit21DoesTestCodeunitExistEi");
extern "C" void agiru_unlinked_1180() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::DoesTestCodeunitExist(int)"); }
extern "C" void agiru_unlinked_1181() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit23AddMissingTestCodeunitsERNS_8platform13Integer_TableENS_4CodeILm10EEE");
extern "C" void agiru_unlinked_1181() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::AddMissingTestCodeunits(agiru::platform::Integer_Table&, agiru::Code<10ul>)"); }
extern "C" void agiru_unlinked_1182() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit25GetTestCodeunitsSelectionENS2_18CALTestSuite_TableE");
extern "C" void agiru_unlinked_1182() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::GetTestCodeunitsSelection(agiru::System::TestTools::TestRunner::CALTestSuite_Table)"); }
extern "C" void agiru_unlinked_1183() asm("_ZN5agiru6System9TestTools10TestRunner26CALTestManagement_Codeunit8RunSuiteERNS2_17CALTestLine_TableEb");
extern "C" void agiru_unlinked_1183() { Unlinked("agiru::System::TestTools::TestRunner::CALTestManagement_Codeunit::RunSuite(agiru::System::TestTools::TestRunner::CALTestLine_Table&, bool)"); }
extern "C" void agiru_unlinked_1184() asm("_ZN5agiru6System9TestTools12CodeCoverage17CodeCoverage_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_1184() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverage_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_1185() asm("_ZN5agiru6System9TestTools12CodeCoverage17CodeCoverage_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_1185() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverage_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_1186() asm("_ZN5agiru6System9TestTools12CodeCoverage17CodeCoverage_Page6OnInitEv");
extern "C" void agiru_unlinked_1186() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverage_Page::OnInit()"); }
extern "C" void agiru_unlinked_1187() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit15ApplicationHitsEv");
extern "C" void agiru_unlinked_1187() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::ApplicationHits()"); }
extern "C" void agiru_unlinked_1188() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit20StartAutomaticBackupEiNS_4TextILm1024EEES5_");
extern "C" void agiru_unlinked_1188() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::StartAutomaticBackup(int, agiru::Text<1024ul>, agiru::Text<1024ul>)"); }
extern "C" void agiru_unlinked_1189() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit23StopApplicationCoverageEv");
extern "C" void agiru_unlinked_1189() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::StopApplicationCoverage()"); }
extern "C" void agiru_unlinked_1190() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit24StartApplicationCoverageEv");
extern "C" void agiru_unlinked_1190() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::StartApplicationCoverage()"); }
extern "C" void agiru_unlinked_1191() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit28GetNoOfHitsCoverageForObjectENS_6OptionIvEEiNS_4TextILm0EEE");
extern "C" void agiru_unlinked_1191() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::GetNoOfHitsCoverageForObject(agiru::Option<void>, int, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1192() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit29UpdateAutomaticBackupSettingsEiNS_4TextILm1024EEES5_");
extern "C" void agiru_unlinked_1192() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::UpdateAutomaticBackupSettings(int, agiru::Text<1024ul>, agiru::Text<1024ul>)"); }
extern "C" void agiru_unlinked_1193() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit4StopEv");
extern "C" void agiru_unlinked_1193() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::Stop()"); }
extern "C" void agiru_unlinked_1194() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit5ClearEv");
extern "C" void agiru_unlinked_1194() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::Clear()"); }
extern "C" void agiru_unlinked_1195() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit5StartEb");
extern "C" void agiru_unlinked_1195() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::Start(bool)"); }
extern "C" void agiru_unlinked_1196() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit6ImportEv");
extern "C" void agiru_unlinked_1196() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::Import()"); }
extern "C" void agiru_unlinked_1197() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit7IncludeERNS_8platform12AllObj_TableE");
extern "C" void agiru_unlinked_1197() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::Include(agiru::platform::AllObj_Table&)"); }
extern "C" void agiru_unlinked_1198() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit7RefreshEv");
extern "C" void agiru_unlinked_1198() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::Refresh()"); }
extern "C" void agiru_unlinked_1199() asm("_ZN5agiru6System9TestTools12CodeCoverage24CodeCoverageMgt_Codeunit7RunningEv");
extern "C" void agiru_unlinked_1199() { Unlinked("agiru::System::TestTools::CodeCoverage::CodeCoverageMgt_Codeunit::Running()"); }
extern "C" void agiru_unlinked_1200() asm("_ZN5agiru6System9Utilities31DotNetExceptionHandler_Codeunit10CastToTypeERNS_6dotnet9ExceptionENS3_4TypeE");
extern "C" void agiru_unlinked_1200() { Unlinked("agiru::System::Utilities::DotNetExceptionHandler_Codeunit::CastToType(agiru::dotnet::Exception&, agiru::dotnet::Type)"); }
extern "C" void agiru_unlinked_1201() asm("_ZN5agiru6System9Utilities31DotNetExceptionHandler_Codeunit10GetMessageEv");
extern "C" void agiru_unlinked_1201() { Unlinked("agiru::System::Utilities::DotNetExceptionHandler_Codeunit::GetMessage()"); }
extern "C" void agiru_unlinked_1202() asm("_ZN5agiru6System9Utilities31DotNetExceptionHandler_Codeunit13TryCastToTypeENS_6dotnet4TypeE");
extern "C" void agiru_unlinked_1202() { Unlinked("agiru::System::Utilities::DotNetExceptionHandler_Codeunit::TryCastToType(agiru::dotnet::Type)"); }
extern "C" void agiru_unlinked_1203() asm("_ZN5agiru6System9Utilities31DotNetExceptionHandler_Codeunit5CatchERNS_6dotnet9ExceptionENS3_4TypeE");
extern "C" void agiru_unlinked_1203() { Unlinked("agiru::System::Utilities::DotNetExceptionHandler_Codeunit::Catch(agiru::dotnet::Exception&, agiru::dotnet::Type)"); }
extern "C" void agiru_unlinked_1204() asm("_ZN5agiru6System9Utilities31DotNetExceptionHandler_Codeunit7CollectEv");
extern "C" void agiru_unlinked_1204() { Unlinked("agiru::System::Utilities::DotNetExceptionHandler_Codeunit::Collect()"); }
extern "C" void agiru_unlinked_1205() asm("_ZN5agiru6System9Utilities31DotNetExceptionHandler_Codeunit7RethrowEv");
extern "C" void agiru_unlinked_1205() { Unlinked("agiru::System::Utilities::DotNetExceptionHandler_Codeunit::Rethrow()"); }
extern "C" void agiru_unlinked_1206() asm("_ZN5agiru7Finance11RoleCenters23AccountPayableCue_Table18GetDefaultWorkDateEv");
extern "C" void agiru_unlinked_1206() { Unlinked("agiru::Finance::RoleCenters::AccountPayableCue_Table::GetDefaultWorkDate()"); }
extern "C" void agiru_unlinked_1207() asm("_ZN5agiru7Finance11RoleCenters26AccPayablePerformance_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_1207() { Unlinked("agiru::Finance::RoleCenters::AccPayablePerformance_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_1208() asm("_ZN5agiru7Finance11RoleCenters30AccPayablePerformance_Codeunit28TopVendorListUpdatedRecentlyERi");
extern "C" void agiru_unlinked_1208() { Unlinked("agiru::Finance::RoleCenters::AccPayablePerformance_Codeunit::TopVendorListUpdatedRecently(int&)"); }
extern "C" void agiru_unlinked_1209() asm("_ZN5agiru7Finance11RoleCenters30AccPayablePerformance_Codeunit32ScheduleTopVendorListRefreshTaskEv");
extern "C" void agiru_unlinked_1209() { Unlinked("agiru::Finance::RoleCenters::AccPayablePerformance_Codeunit::ScheduleTopVendorListRefreshTask()"); }
extern "C" void agiru_unlinked_1210() asm("_ZN5agiru7Finance13Consolidation21BusinessUnitCard_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_1210() { Unlinked("agiru::Finance::Consolidation::BusinessUnitCard_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_1211() asm("_ZN5agiru7Finance13Consolidation21BusinessUnitCard_Page18OnValidateBCAPIURLEv");
extern "C" void agiru_unlinked_1211() { Unlinked("agiru::Finance::Consolidation::BusinessUnitCard_Page::OnValidateBCAPIURL()"); }
extern "C" void agiru_unlinked_1212() asm("_ZN5agiru7Finance13Consolidation21BusinessUnitCard_Page19OnVisibleDBSettingsEv");
extern "C" void agiru_unlinked_1212() { Unlinked("agiru::Finance::Consolidation::BusinessUnitCard_Page::OnVisibleDBSettings()"); }
extern "C" void agiru_unlinked_1213() asm("_ZN5agiru7Finance13Consolidation21BusinessUnitCard_Page20OnActionTestDatabaseEv");
extern "C" void agiru_unlinked_1213() { Unlinked("agiru::Finance::Consolidation::BusinessUnitCard_Page::OnActionTestDatabase()"); }
extern "C" void agiru_unlinked_1214() asm("_ZN5agiru7Finance13Consolidation21BusinessUnitCard_Page20OnVisibleAPISettingsEv");
extern "C" void agiru_unlinked_1214() { Unlinked("agiru::Finance::Consolidation::BusinessUnitCard_Page::OnVisibleAPISettings()"); }
extern "C" void agiru_unlinked_1215() asm("_ZN5agiru7Finance13Consolidation21BusinessUnitCard_Page24OnActionRunConsolidationEv");
extern "C" void agiru_unlinked_1215() { Unlinked("agiru::Finance::Consolidation::BusinessUnitCard_Page::OnActionRunConsolidation()"); }
extern "C" void agiru_unlinked_1216() asm("_ZN5agiru7Finance13Consolidation21BusinessUnitCard_Page30OnActionConfigureExchangeRatesEv");
extern "C" void agiru_unlinked_1216() { Unlinked("agiru::Finance::Consolidation::BusinessUnitCard_Page::OnActionConfigureExchangeRates()"); }
extern "C" void agiru_unlinked_1217() asm("_ZN5agiru7Finance13Consolidation21BusinessUnitCard_Page32OnVisibleDefaultDataImportMethodEv");
extern "C" void agiru_unlinked_1217() { Unlinked("agiru::Finance::Consolidation::BusinessUnitCard_Page::OnVisibleDefaultDataImportMethod()"); }
extern "C" void agiru_unlinked_1218() asm("_ZN5agiru7Finance13Consolidation21BusinessUnitCard_Page33OnValidateDefaultDataImportMethodEv");
extern "C" void agiru_unlinked_1218() { Unlinked("agiru::Finance::Consolidation::BusinessUnitCard_Page::OnValidateDefaultDataImportMethod()"); }
extern "C" void agiru_unlinked_1219() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_1219() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_1220() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page14OnVisibleStep0Ev");
extern "C" void agiru_unlinked_1220() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleStep0()"); }
extern "C" void agiru_unlinked_1221() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page14OnVisibleStep1Ev");
extern "C" void agiru_unlinked_1221() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleStep1()"); }
extern "C" void agiru_unlinked_1222() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page14OnVisibleStep2Ev");
extern "C" void agiru_unlinked_1222() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleStep2()"); }
extern "C" void agiru_unlinked_1223() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page14OnVisibleStep3Ev");
extern "C" void agiru_unlinked_1223() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleStep3()"); }
extern "C" void agiru_unlinked_1224() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page16OnActionFinalizeEv");
extern "C" void agiru_unlinked_1224() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnActionFinalize()"); }
extern "C" void agiru_unlinked_1225() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_1225() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_1226() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page17OnVisibleFinalizeEv");
extern "C" void agiru_unlinked_1226() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleFinalize()"); }
extern "C" void agiru_unlinked_1227() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page18OnActionActionBackEv");
extern "C" void agiru_unlinked_1227() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnActionActionBack()"); }
extern "C" void agiru_unlinked_1228() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page18OnActionActionNextEv");
extern "C" void agiru_unlinked_1228() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnActionActionNext()"); }
extern "C" void agiru_unlinked_1229() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page19OnActionGrantAccessEv");
extern "C" void agiru_unlinked_1229() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnActionGrantAccess()"); }
extern "C" void agiru_unlinked_1230() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page19OnEnabledActionBackEv");
extern "C" void agiru_unlinked_1230() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnEnabledActionBack()"); }
extern "C" void agiru_unlinked_1231() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page19OnEnabledActionNextEv");
extern "C" void agiru_unlinked_1231() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnEnabledActionNext()"); }
extern "C" void agiru_unlinked_1232() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page19OnVisibleActionNextEv");
extern "C" void agiru_unlinked_1232() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleActionNext()"); }
extern "C" void agiru_unlinked_1233() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page19OnVisibleDocumentNoEv");
extern "C" void agiru_unlinked_1233() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleDocumentNo()"); }
extern "C" void agiru_unlinked_1234() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page20OnValidateDocumentNoEv");
extern "C" void agiru_unlinked_1234() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnValidateDocumentNo()"); }
extern "C" void agiru_unlinked_1235() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page20OnValidateEndingDateEv");
extern "C" void agiru_unlinked_1235() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnValidateEndingDate()"); }
extern "C" void agiru_unlinked_1236() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page20OnVisibleGrantAccessEv");
extern "C" void agiru_unlinked_1236() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleGrantAccess()"); }
extern "C" void agiru_unlinked_1237() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page21OnValidateConsolidateEv");
extern "C" void agiru_unlinked_1237() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnValidateConsolidate()"); }
extern "C" void agiru_unlinked_1238() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page22OnValidateStartingDateEv");
extern "C" void agiru_unlinked_1238() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnValidateStartingDate()"); }
extern "C" void agiru_unlinked_1239() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page24OnLookupJournalBatchNameERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_1239() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnLookupJournalBatchName(agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_1240() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page25OnActionConfigureCurrencyEv");
extern "C" void agiru_unlinked_1240() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnActionConfigureCurrency()"); }
extern "C" void agiru_unlinked_1241() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page25OnVisibleJournalBatchNameEv");
extern "C" void agiru_unlinked_1241() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleJournalBatchName()"); }
extern "C" void agiru_unlinked_1242() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page26OnValidateJournalBatchNameEv");
extern "C" void agiru_unlinked_1242() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnValidateJournalBatchName()"); }
extern "C" void agiru_unlinked_1243() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page26OnVisibleConfigureCurrencyEv");
extern "C" void agiru_unlinked_1243() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleConfigureCurrency()"); }
extern "C" void agiru_unlinked_1244() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page28OnVisibleJournalTemplateNameEv");
extern "C" void agiru_unlinked_1244() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnVisibleJournalTemplateName()"); }
extern "C" void agiru_unlinked_1245() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page29OnValidateJournalTemplateNameEv");
extern "C" void agiru_unlinked_1245() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnValidateJournalTemplateName()"); }
extern "C" void agiru_unlinked_1246() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page31OnEditableCurrencyBusinessUnitsEv");
extern "C" void agiru_unlinked_1246() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnEditableCurrencyBusinessUnits()"); }
extern "C" void agiru_unlinked_1247() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page32OnAssistEditTransferedDimensionsEv");
extern "C" void agiru_unlinked_1247() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnAssistEditTransferedDimensions()"); }
extern "C" void agiru_unlinked_1248() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page32OnDrillDownAverageCurrencyFactorEv");
extern "C" void agiru_unlinked_1248() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnDrillDownAverageCurrencyFactor()"); }
extern "C" void agiru_unlinked_1249() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page32OnDrillDownClosingCurrencyFactorEv");
extern "C" void agiru_unlinked_1249() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnDrillDownClosingCurrencyFactor()"); }
extern "C" void agiru_unlinked_1250() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page36OnDrillDownLastClosingCurrencyFactorEv");
extern "C" void agiru_unlinked_1250() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnDrillDownLastClosingCurrencyFactor()"); }
extern "C" void agiru_unlinked_1251() asm("_ZN5agiru7Finance13Consolidation22ConsolidateWizard_Page38OnDrillDownLastConsolidationEndingDateEv");
extern "C" void agiru_unlinked_1251() { Unlinked("agiru::Finance::Consolidation::ConsolidateWizard_Page::OnDrillDownLastConsolidationEndingDate()"); }
extern "C" void agiru_unlinked_1252() asm("_ZN5agiru7Finance16FinancialReports24TrialBalanceMgt_Codeunit10NextPeriodENS_7AlArrayINS_4TextILm0EEELm0EEENS3_INS3_INS_7DecimalELm2EEELm0EEES6_i");
extern "C" void agiru_unlinked_1252() { Unlinked("agiru::Finance::FinancialReports::TrialBalanceMgt_Codeunit::NextPeriod(agiru::AlArray<agiru::Text<0ul>, 0ul>, agiru::AlArray<agiru::AlArray<agiru::Decimal, 2ul>, 0ul>, agiru::AlArray<agiru::Text<0ul>, 0ul>, int)"); }
extern "C" void agiru_unlinked_1253() asm("_ZN5agiru7Finance16FinancialReports24TrialBalanceMgt_Codeunit14PreviousPeriodENS_7AlArrayINS_4TextILm0EEELm0EEENS3_INS3_INS_7DecimalELm2EEELm0EEES6_i");
extern "C" void agiru_unlinked_1253() { Unlinked("agiru::Finance::FinancialReports::TrialBalanceMgt_Codeunit::PreviousPeriod(agiru::AlArray<agiru::Text<0ul>, 0ul>, agiru::AlArray<agiru::AlArray<agiru::Decimal, 2ul>, 0ul>, agiru::AlArray<agiru::Text<0ul>, 0ul>, int)"); }
extern "C" void agiru_unlinked_1254() asm("_ZN5agiru7Finance16FinancialReports24TrialBalanceMgt_Codeunit14SetupIsInPlaceEv");
extern "C" void agiru_unlinked_1254() { Unlinked("agiru::Finance::FinancialReports::TrialBalanceMgt_Codeunit::SetupIsInPlace()"); }
extern "C" void agiru_unlinked_1255() asm("_ZN5agiru7Finance16FinancialReports24TrialBalanceMgt_Codeunit8LoadDataENS_7AlArrayINS_4TextILm0EEELm0EEENS3_INS3_INS_7DecimalELm2EEELm0EEES6_i");
extern "C" void agiru_unlinked_1255() { Unlinked("agiru::Finance::FinancialReports::TrialBalanceMgt_Codeunit::LoadData(agiru::AlArray<agiru::Text<0ul>, 0ul>, agiru::AlArray<agiru::AlArray<agiru::Decimal, 2ul>, 0ul>, agiru::AlArray<agiru::Text<0ul>, 0ul>, int)"); }
extern "C" void agiru_unlinked_1256() asm("_ZN5agiru7Finance16FinancialReports24TrialBalanceMgt_Codeunit9DrillDownEii");
extern "C" void agiru_unlinked_1256() { Unlinked("agiru::Finance::FinancialReports::TrialBalanceMgt_Codeunit::DrillDown(int, int)"); }
extern "C" void agiru_unlinked_1257() asm("_ZN5agiru7Finance16FinancialReports28FinReportExcelTemplates_Page9SetSourceERNS1_21FinancialReport_TableERNS1_21AccScheduleLine_TableE");
extern "C" void agiru_unlinked_1257() { Unlinked("agiru::Finance::FinancialReports::FinReportExcelTemplates_Page::SetSource(agiru::Finance::FinancialReports::FinancialReport_Table&, agiru::Finance::FinancialReports::AccScheduleLine_Table&)"); }
extern "C" void agiru_unlinked_1258() asm("_ZN5agiru7Finance19ReceivablesPayables25AccPayablePerfCharts_Page16OnQueryClosePageENS_6ActionE");
extern "C" void agiru_unlinked_1258() { Unlinked("agiru::Finance::ReceivablesPayables::AccPayablePerfCharts_Page::OnQueryClosePage(agiru::Action)"); }
extern "C" void agiru_unlinked_1259() asm("_ZN5agiru7Service8Document22ServiceStatistics_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_1259() { Unlinked("agiru::Service::Document::ServiceStatistics_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_1260() asm("_ZN5agiru7Service8Document22ServiceStatistics_Page15OnValidateTotalEv");
extern "C" void agiru_unlinked_1260() { Unlinked("agiru::Service::Document::ServiceStatistics_Page::OnValidateTotal()"); }
extern "C" void agiru_unlinked_1261() asm("_ZN5agiru7Service8Document22ServiceStatistics_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_1261() { Unlinked("agiru::Service::Document::ServiceStatistics_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_1262() asm("_ZN5agiru7Service8Document22ServiceStatistics_Page16OnQueryClosePageENS_6ActionE");
extern "C" void agiru_unlinked_1262() { Unlinked("agiru::Service::Document::ServiceStatistics_Page::OnQueryClosePage(agiru::Action)"); }
extern "C" void agiru_unlinked_1263() asm("_ZN5agiru7Service8Document22ServiceStatistics_Page23OnValidateTotalAmount11Ev");
extern "C" void agiru_unlinked_1263() { Unlinked("agiru::Service::Document::ServiceStatistics_Page::OnValidateTotalAmount11()"); }
extern "C" void agiru_unlinked_1264() asm("_ZN5agiru7Service8Document22ServiceStatistics_Page23OnValidateTotalAmount16Ev");
extern "C" void agiru_unlinked_1264() { Unlinked("agiru::Service::Document::ServiceStatistics_Page::OnValidateTotalAmount16()"); }
extern "C" void agiru_unlinked_1265() asm("_ZN5agiru7Service8Document22ServiceStatistics_Page23OnValidateTotalAmount17Ev");
extern "C" void agiru_unlinked_1265() { Unlinked("agiru::Service::Document::ServiceStatistics_Page::OnValidateTotalAmount17()"); }
extern "C" void agiru_unlinked_1266() asm("_ZN5agiru7Service8Document22ServiceStatistics_Page35OnValidateInvDiscountAmount_GeneralEv");
extern "C" void agiru_unlinked_1266() { Unlinked("agiru::Service::Document::ServiceStatistics_Page::OnValidateInvDiscountAmount_General()"); }
extern "C" void agiru_unlinked_1267() asm("_ZN5agiru7Service8Document22ServiceStatistics_Page55OnDrillDownTotalAdjCostLCY1TotalServLineLCY1UnitCostLCYEv");
extern "C" void agiru_unlinked_1267() { Unlinked("agiru::Service::Document::ServiceStatistics_Page::OnDrillDownTotalAdjCostLCY1TotalServLineLCY1UnitCostLCY()"); }
extern "C" void agiru_unlinked_1268() asm("_ZN5agiru7Service8Document22ServiceStatistics_Page55OnDrillDownTotalAdjCostLCY5TotalServLineLCY5UnitCostLCYEv");
extern "C" void agiru_unlinked_1268() { Unlinked("agiru::Service::Document::ServiceStatistics_Page::OnDrillDownTotalAdjCostLCY5TotalServLineLCY5UnitCostLCY()"); }
extern "C" void agiru_unlinked_1269() asm("_ZN5agiru7Upgrade23UpgradeBaseApp_Codeunit23UpgradeSEPACAMT05300108Ev");
extern "C" void agiru_unlinked_1269() { Unlinked("agiru::Upgrade::UpgradeBaseApp_Codeunit::UpgradeSEPACAMT05300108()"); }
extern "C" void agiru_unlinked_1270() asm("_ZN5agiru7Upgrade23UpgradeBaseApp_Codeunit25UpgradeWordTemplateTablesEv");
extern "C" void agiru_unlinked_1270() { Unlinked("agiru::Upgrade::UpgradeBaseApp_Codeunit::UpgradeWordTemplateTables()"); }
extern "C" void agiru_unlinked_1271() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page11OnActionDayEv");
extern "C" void agiru_unlinked_1271() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionDay()"); }
extern "C" void agiru_unlinked_1272() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page12OnActionWeekEv");
extern "C" void agiru_unlinked_1272() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionWeek()"); }
extern "C" void agiru_unlinked_1273() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page12OnActionYearEv");
extern "C" void agiru_unlinked_1273() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionYear()"); }
extern "C" void agiru_unlinked_1274() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page13OnActionMonthEv");
extern "C" void agiru_unlinked_1274() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionMonth()"); }
extern "C" void agiru_unlinked_1275() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page13OnVisibleShowEv");
extern "C" void agiru_unlinked_1275() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleShow()"); }
extern "C" void agiru_unlinked_1276() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page14OnActionPosNegEv");
extern "C" void agiru_unlinked_1276() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionPosNeg()"); }
extern "C" void agiru_unlinked_1277() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page15OnActionAccountEv");
extern "C" void agiru_unlinked_1277() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionAccount()"); }
extern "C" void agiru_unlinked_1278() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page15OnActionQuarterEv");
extern "C" void agiru_unlinked_1278() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionQuarter()"); }
extern "C" void agiru_unlinked_1279() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page16OnActionCombinedEv");
extern "C" void agiru_unlinked_1279() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionCombined()"); }
extern "C" void agiru_unlinked_1280() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page16OnActionWorkDateEv");
extern "C" void agiru_unlinked_1280() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionWorkDate()"); }
extern "C" void agiru_unlinked_1281() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page16OnVisibleGroupByEv");
extern "C" void agiru_unlinked_1281() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleGroupBy()"); }
extern "C" void agiru_unlinked_1282() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page18OnActionSourceTypeEv");
extern "C" void agiru_unlinked_1282() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionSourceType()"); }
extern "C" void agiru_unlinked_1283() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page18OnVisibleStartDateEv");
extern "C" void agiru_unlinked_1283() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleStartDate()"); }
extern "C" void agiru_unlinked_1284() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page19OnVisibleDisclaimerEv");
extern "C" void agiru_unlinked_1284() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleDisclaimer()"); }
extern "C" void agiru_unlinked_1285() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page19OnVisibleStatusTextEv");
extern "C" void agiru_unlinked_1285() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleStatusText()"); }
extern "C" void agiru_unlinked_1286() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page20OnActionChangeInCashEv");
extern "C" void agiru_unlinked_1286() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionChangeInCash()"); }
extern "C" void agiru_unlinked_1287() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page20OnVisibleNotSetupLblEv");
extern "C" void agiru_unlinked_1287() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleNotSetupLbl()"); }
extern "C" void agiru_unlinked_1288() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page21OnVisibleChartOptionsEv");
extern "C" void agiru_unlinked_1288() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleChartOptions()"); }
extern "C" void agiru_unlinked_1289() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page21OnVisiblePeriodLengthEv");
extern "C" void agiru_unlinked_1289() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisiblePeriodLength()"); }
extern "C" void agiru_unlinked_1290() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page22OnActionFisrtEntryDateEv");
extern "C" void agiru_unlinked_1290() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionFisrtEntryDate()"); }
extern "C" void agiru_unlinked_1291() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page22OnVisibleBusinessChartEv");
extern "C" void agiru_unlinked_1291() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleBusinessChart()"); }
extern "C" void agiru_unlinked_1292() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page23OnActionAccumulatedCashEv");
extern "C" void agiru_unlinked_1292() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionAccumulatedCash()"); }
extern "C" void agiru_unlinked_1293() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page24OnActionChartInformationEv");
extern "C" void agiru_unlinked_1293() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionChartInformation()"); }
extern "C" void agiru_unlinked_1294() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page25OnActionOpenAssistedSetupEv");
extern "C" void agiru_unlinked_1294() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionOpenAssistedSetup()"); }
extern "C" void agiru_unlinked_1295() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page25OnVisibleManualAdjustmentEv");
extern "C" void agiru_unlinked_1295() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleManualAdjustment()"); }
extern "C" void agiru_unlinked_1296() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page26OnActionEditManualExpensesEv");
extern "C" void agiru_unlinked_1296() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionEditManualExpenses()"); }
extern "C" void agiru_unlinked_1297() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page26OnActionEditManualRevenuesEv");
extern "C" void agiru_unlinked_1297() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionEditManualRevenues()"); }
extern "C" void agiru_unlinked_1298() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page26OnVisibleOpenAssistedSetupEv");
extern "C" void agiru_unlinked_1298() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleOpenAssistedSetup()"); }
extern "C" void agiru_unlinked_1299() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page27OnActionRecalculateForecastEv");
extern "C" void agiru_unlinked_1299() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnActionRecalculateForecast()"); }
extern "C" void agiru_unlinked_1300() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page27OnVisibleEditManualExpensesEv");
extern "C" void agiru_unlinked_1300() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleEditManualExpenses()"); }
extern "C" void agiru_unlinked_1301() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page27OnVisibleEditManualRevenuesEv");
extern "C" void agiru_unlinked_1301() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleEditManualRevenues()"); }
extern "C" void agiru_unlinked_1302() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page28OnVisibleRecalculateForecastEv");
extern "C" void agiru_unlinked_1302() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnVisibleRecalculateForecast()"); }
extern "C" void agiru_unlinked_1303() asm("_ZN5agiru8CashFlow8Forecast26CashFlowForecastChart_Page6OnInitEv");
extern "C" void agiru_unlinked_1303() { Unlinked("agiru::CashFlow::Forecast::CashFlowForecastChart_Page::OnInit()"); }
extern "C" void agiru_unlinked_1304() asm("_ZN5agiru8CashFlow8Forecast30ClientDetailCashFlowChart_Page6OnInitEv");
extern "C" void agiru_unlinked_1304() { Unlinked("agiru::CashFlow::Forecast::ClientDetailCashFlowChart_Page::OnInit()"); }
extern "C" void agiru_unlinked_1305() asm("_ZN5agiru8Projects7Project8Planning32JobPlanningLineCalendar_Codeunit13CreateAndSendEv");
extern "C" void agiru_unlinked_1305() { Unlinked("agiru::Projects::Project::Planning::JobPlanningLineCalendar_Codeunit::CreateAndSend()"); }
extern "C" void agiru_unlinked_1306() asm("_ZN5agiru8Projects7Project8Planning32JobPlanningLineCalendar_Codeunit13CreateRequestERNS_6System5Email15EmailItem_TableE");
extern "C" void agiru_unlinked_1306() { Unlinked("agiru::Projects::Project::Planning::JobPlanningLineCalendar_Codeunit::CreateRequest(agiru::System::Email::EmailItem_Table&)"); }
extern "C" void agiru_unlinked_1307() asm("_ZN5agiru8Projects7Project8Planning32JobPlanningLineCalendar_Codeunit15SetPlanningLineENS2_21JobPlanningLine_TableE");
extern "C" void agiru_unlinked_1307() { Unlinked("agiru::Projects::Project::Planning::JobPlanningLineCalendar_Codeunit::SetPlanningLine(agiru::Projects::Project::Planning::JobPlanningLine_Table)"); }
extern "C" void agiru_unlinked_1308() asm("_ZN5agiru8Projects7Project8Planning32JobPlanningLineCalendar_Codeunit18CreateCancellationERNS_6System5Email15EmailItem_TableE");
extern "C" void agiru_unlinked_1308() { Unlinked("agiru::Projects::Project::Planning::JobPlanningLineCalendar_Codeunit::CreateCancellation(agiru::System::Email::EmailItem_Table&)"); }
extern "C" void agiru_unlinked_1309() asm("_ZN5agiru8Projects9TimeSheet19TimeSheetChart_Page20OnAfterGetCurrRecordEv");
extern "C" void agiru_unlinked_1309() { Unlinked("agiru::Projects::TimeSheet::TimeSheetChart_Page::OnAfterGetCurrRecord()"); }
extern "C" void agiru_unlinked_1310() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit10RsoRequestENS_4TextILm0EEENS_4CodeILm6EEES4_RS4_S7_S7_Ri");
extern "C" void agiru_unlinked_1310() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::RsoRequest(agiru::Text<0ul>, agiru::Code<6ul>, agiru::Text<0ul>, agiru::Text<0ul>&, agiru::Text<0ul>&, agiru::Text<0ul>&, int&)"); }
extern "C" void agiru_unlinked_1311() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit11StartUploadEi");
extern "C" void agiru_unlinked_1311() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::StartUpload(int)"); }
extern "C" void agiru_unlinked_1312() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit12GetDocumentsENS_4TextILm0EEE");
extern "C" void agiru_unlinked_1312() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::GetDocuments(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1313() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit14TestConnectionERNS1_21OCRServiceSetup_TableE");
extern "C" void agiru_unlinked_1313() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::TestConnection(agiru::EServices::EDocument::OCRServiceSetup_Table&)"); }
extern "C" void agiru_unlinked_1314() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit15SetupConnectionERNS1_21OCRServiceSetup_TableE");
extern "C" void agiru_unlinked_1314() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::SetupConnection(agiru::EServices::EDocument::OCRServiceSetup_Table&)"); }
extern "C" void agiru_unlinked_1315() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit16CheckCredentialsEv");
extern "C" void agiru_unlinked_1315() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::CheckCredentials()"); }
extern "C" void agiru_unlinked_1316() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit16UploadAttachmentERNS_6System9Utilities17TempBlob_CodeunitENS_4TextILm0EEENS7_ILm50EEENS_4CodeILm20EEENS_8RecordIdE");
extern "C" void agiru_unlinked_1316() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::UploadAttachment(agiru::System::Utilities::TempBlob_Codeunit&, agiru::Text<0ul>, agiru::Text<50ul>, agiru::Code<20ul>, agiru::RecordId)"); }
extern "C" void agiru_unlinked_1317() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit17LogActivityFailedENS_8RecordIdENS_4TextILm0EEES5_");
extern "C" void agiru_unlinked_1317() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::LogActivityFailed(agiru::RecordId, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1318() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit18GetStatusHyperLinkENS1_22IncomingDocument_TableE");
extern "C" void agiru_unlinked_1318() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::GetStatusHyperLink(agiru::EServices::EDocument::IncomingDocument_Table)"); }
extern "C" void agiru_unlinked_1319() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit18OcrServiceIsEnableEv");
extern "C" void agiru_unlinked_1319() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::OcrServiceIsEnable()"); }
extern "C" void agiru_unlinked_1320() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit19SetURLsToDefaultRSOERNS1_21OCRServiceSetup_TableE");
extern "C" void agiru_unlinked_1320() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::SetURLsToDefaultRSO(agiru::EServices::EDocument::OCRServiceSetup_Table&)"); }
extern "C" void agiru_unlinked_1321() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit20LogActivitySucceededENS_8RecordIdENS_4TextILm0EEES5_");
extern "C" void agiru_unlinked_1321() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::LogActivitySucceeded(agiru::RecordId, agiru::Text<0ul>, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1322() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit21GetCredentialsErrTextEv");
extern "C" void agiru_unlinked_1322() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::GetCredentialsErrText()"); }
extern "C" void agiru_unlinked_1323() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit22UpdateOrganizationInfoERNS1_21OCRServiceSetup_TableE");
extern "C" void agiru_unlinked_1323() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::UpdateOrganizationInfo(agiru::EServices::EDocument::OCRServiceSetup_Table&)"); }
extern "C" void agiru_unlinked_1324() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit22UploadCorrectedOCRFileENS1_22IncomingDocument_TableE");
extern "C" void agiru_unlinked_1324() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::UploadCorrectedOCRFile(agiru::EServices::EDocument::IncomingDocument_Table)"); }
extern "C" void agiru_unlinked_1325() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit23DateConvertYYYYMMDD2XMLENS_4TextILm0EEE");
extern "C" void agiru_unlinked_1325() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::DateConvertYYYYMMDD2XML(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1326() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit23GetFeatureTelemetryNameEv");
extern "C" void agiru_unlinked_1326() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::GetFeatureTelemetryName()"); }
extern "C" void agiru_unlinked_1327() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit24GetDocumentForAttachmentERNS1_32IncomingDocumentAttachment_TableE");
extern "C" void agiru_unlinked_1327() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::GetDocumentForAttachment(agiru::EServices::EDocument::IncomingDocumentAttachment_Table&)"); }
extern "C" void agiru_unlinked_1328() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit25GetOriginalOCRXMLRootNodeENS1_22IncomingDocument_TableERNS_6dotnet7XmlNodeE");
extern "C" void agiru_unlinked_1328() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::GetOriginalOCRXMLRootNode(agiru::EServices::EDocument::IncomingDocument_Table, agiru::dotnet::XmlNode&)"); }
extern "C" void agiru_unlinked_1329() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit26GetOcrServiceSetupExtendedERNS1_21OCRServiceSetup_TableEb");
extern "C" void agiru_unlinked_1329() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::GetOcrServiceSetupExtended(agiru::EServices::EDocument::OCRServiceSetup_Table&, bool)"); }
extern "C" void agiru_unlinked_1330() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit26UpdateOcrDocumentTemplatesEv");
extern "C" void agiru_unlinked_1330() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::UpdateOcrDocumentTemplates()"); }
extern "C" void agiru_unlinked_1331() asm("_ZN5agiru9EServices9EDocument22OCRServiceMgt_Codeunit28UpdateIncomingDocWithOCRDataERNS1_22IncomingDocument_TableERNS_6dotnet7XmlNodeE");
extern "C" void agiru_unlinked_1331() { Unlinked("agiru::EServices::EDocument::OCRServiceMgt_Codeunit::UpdateIncomingDocWithOCRData(agiru::EServices::EDocument::IncomingDocument_Table&, agiru::dotnet::XmlNode&)"); }
extern "C" void agiru_unlinked_1332() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit10RenewTokenEb");
extern "C" void agiru_unlinked_1332() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::RenewToken(bool)"); }
extern "C" void agiru_unlinked_1333() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit11GetClientIdEb");
extern "C" void agiru_unlinked_1333() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::GetClientId(bool)"); }
extern "C" void agiru_unlinked_1334() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit12SendDocumentENS_7VariantERNS_6System9Utilities17TempBlob_CodeunitE");
extern "C" void agiru_unlinked_1334() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::SendDocument(agiru::Variant, agiru::System::Utilities::TempBlob_Codeunit&)"); }
extern "C" void agiru_unlinked_1335() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit15CheckConnectionEv");
extern "C" void agiru_unlinked_1335() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::CheckConnection()"); }
extern "C" void agiru_unlinked_1336() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit16ReceiveDocumentsENS_8RecordIdE");
extern "C" void agiru_unlinked_1336() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::ReceiveDocuments(agiru::RecordId)"); }
extern "C" void agiru_unlinked_1337() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit16SetURLsToDefaultERNS1_25DocExchServiceSetup_TableEb");
extern "C" void agiru_unlinked_1337() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::SetURLsToDefault(agiru::EServices::EDocument::DocExchServiceSetup_Table&, bool)"); }
extern "C" void agiru_unlinked_1338() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit17GetDocumentStatusENS_8RecordIdENS_4TextILm50EEES5_");
extern "C" void agiru_unlinked_1338() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::GetDocumentStatus(agiru::RecordId, agiru::Text<50ul>, agiru::Text<50ul>)"); }
extern "C" void agiru_unlinked_1339() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit17GetExternalDocURLENS_4TextILm0EEE");
extern "C" void agiru_unlinked_1339() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::GetExternalDocURL(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1340() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit18GetPostSalesInvURLEv");
extern "C" void agiru_unlinked_1340() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::GetPostSalesInvURL()"); }
extern "C" void agiru_unlinked_1341() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit19CheckServiceEnabledEv");
extern "C" void agiru_unlinked_1341() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::CheckServiceEnabled()"); }
extern "C" void agiru_unlinked_1342() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit19VerifyPrerequisitesEb");
extern "C" void agiru_unlinked_1342() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::VerifyPrerequisites(bool)"); }
extern "C" void agiru_unlinked_1343() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit21GetDefaultRedirectUrlEv");
extern "C" void agiru_unlinked_1343() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::GetDefaultRedirectUrl()"); }
extern "C" void agiru_unlinked_1344() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit21GetPostSalesCrMemoURLEv");
extern "C" void agiru_unlinked_1344() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::GetPostSalesCrMemoURL()"); }
extern "C" void agiru_unlinked_1345() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit21SetDefaultRedirectUrlERNS1_25DocExchServiceSetup_TableE");
extern "C" void agiru_unlinked_1345() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::SetDefaultRedirectUrl(agiru::EServices::EDocument::DocExchServiceSetup_Table&)"); }
extern "C" void agiru_unlinked_1346() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit23GetFeatureTelemetryNameEv");
extern "C" void agiru_unlinked_1346() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::GetFeatureTelemetryName()"); }
extern "C" void agiru_unlinked_1347() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit25HasPredefinedOAuth2ParamsEv");
extern "C" void agiru_unlinked_1347() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::HasPredefinedOAuth2Params()"); }
extern "C" void agiru_unlinked_1348() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit26SendRenewTokenNotificationEv");
extern "C" void agiru_unlinked_1348() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::SendRenewTokenNotification()"); }
extern "C" void agiru_unlinked_1349() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit27SendActivateAppNotificationEv");
extern "C" void agiru_unlinked_1349() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::SendActivateAppNotification()"); }
extern "C" void agiru_unlinked_1350() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit29RecallActivateAppNotificationEv");
extern "C" void agiru_unlinked_1350() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::RecallActivateAppNotification()"); }
extern "C" void agiru_unlinked_1351() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit37AcquireAccessTokenByAuthorizationCodeEb");
extern "C" void agiru_unlinked_1351() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::AcquireAccessTokenByAuthorizationCode(bool)"); }
extern "C" void agiru_unlinked_1352() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit9GetAppUrlERNS1_25DocExchServiceSetup_TableE");
extern "C" void agiru_unlinked_1352() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::GetAppUrl(agiru::EServices::EDocument::DocExchServiceSetup_Table&)"); }
extern "C" void agiru_unlinked_1353() asm("_ZN5agiru9EServices9EDocument26DocExchServiceMgt_Codeunit9IsSandboxERNS1_25DocExchServiceSetup_TableE");
extern "C" void agiru_unlinked_1353() { Unlinked("agiru::EServices::EDocument::DocExchServiceMgt_Codeunit::IsSandbox(agiru::EServices::EDocument::DocExchServiceSetup_Table&)"); }
extern "C" void agiru_unlinked_1354() asm("_ZN5agiru9EServices9EDocument30IncomingDocumentApprovers_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_1354() { Unlinked("agiru::EServices::EDocument::IncomingDocumentApprovers_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_1355() asm("_ZN5agiru9EServices9EDocument30IncomingDocumentApprovers_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_1355() { Unlinked("agiru::EServices::EDocument::IncomingDocumentApprovers_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_1356() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit12IsConfiguredEv");
extern "C" void agiru_unlinked_1356() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::IsConfigured()"); }
extern "C" void agiru_unlinked_1357() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit12IsServiceUriENS_4TextILm0EEE");
extern "C" void agiru_unlinked_1357() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::IsServiceUri(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1358() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit12OpenDocumentENS_4TextILm0EEE");
extern "C" void agiru_unlinked_1358() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::OpenDocument(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1359() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit14EditInOneDriveENS_4TextILm0EEES4_NS_4EnumINS_6System11Integration31DocSharingConflictBehavior_EnumEEERNS6_9Utilities17TempBlob_CodeunitE");
extern "C" void agiru_unlinked_1359() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::EditInOneDrive(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Enum<agiru::System::Integration::DocSharingConflictBehavior_Enum>, agiru::System::Utilities::TempBlob_Codeunit&)"); }
extern "C" void agiru_unlinked_1360() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit14EditInOneDriveENS_4TextILm0EEES4_RNS_6System9Utilities17TempBlob_CodeunitE");
extern "C" void agiru_unlinked_1360() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::EditInOneDrive(agiru::Text<0ul>, agiru::Text<0ul>, agiru::System::Utilities::TempBlob_Codeunit&)"); }
extern "C" void agiru_unlinked_1361() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit14GetServiceTypeEv");
extern "C" void agiru_unlinked_1361() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::GetServiceType()"); }
extern "C" void agiru_unlinked_1362() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit14OpenInOneDriveENS_4TextILm0EEES4_NS_8InStreamE");
extern "C" void agiru_unlinked_1362() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::OpenInOneDrive(agiru::Text<0ul>, agiru::Text<0ul>, agiru::InStream)"); }
extern "C" void agiru_unlinked_1363() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit14SetServiceTypeENS_4TextILm0EEE");
extern "C" void agiru_unlinked_1363() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::SetServiceType(agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1364() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit14TestConnectionEv");
extern "C" void agiru_unlinked_1364() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::TestConnection()"); }
extern "C" void agiru_unlinked_1365() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit17ShareWithOneDriveENS_4TextILm0EEES4_NS_8InStreamE");
extern "C" void agiru_unlinked_1365() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::ShareWithOneDrive(agiru::Text<0ul>, agiru::Text<0ul>, agiru::InStream)"); }
extern "C" void agiru_unlinked_1366() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit19GetOneDriveScenarioERNS_6absent23DocumentServiceScenarioE");
extern "C" void agiru_unlinked_1366() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::GetOneDriveScenario(agiru::absent::DocumentServiceScenario&)"); }
extern "C" void agiru_unlinked_1367() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit20TestLocationResolvesENS_4TextILm250EEENS_10SecretTextE");
extern "C" void agiru_unlinked_1367() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::TestLocationResolves(agiru::Text<250ul>, agiru::SecretText)"); }
extern "C" void agiru_unlinked_1368() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit21TryGetDefaultLocationERNS_4TextILm0EEE");
extern "C" void agiru_unlinked_1368() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::TryGetDefaultLocation(agiru::Text<0ul>&)"); }
extern "C" void agiru_unlinked_1369() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit23EditInOneDriveFromMediaENS_4TextILm0EEES4_NS_4GuidE");
extern "C" void agiru_unlinked_1369() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::EditInOneDriveFromMedia(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Guid)"); }
extern "C" void agiru_unlinked_1370() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit23OpenInOneDriveFromMediaENS_4TextILm0EEES4_NS_4GuidE");
extern "C" void agiru_unlinked_1370() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::OpenInOneDriveFromMedia(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Guid)"); }
extern "C" void agiru_unlinked_1371() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit26ShareWithOneDriveFromMediaENS_4TextILm0EEES4_NS_4GuidE");
extern "C" void agiru_unlinked_1371() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::ShareWithOneDriveFromMedia(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Guid)"); }
extern "C" void agiru_unlinked_1372() asm("_ZN5agiru9EServices9EDocument34DocumentServiceManagement_Codeunit8SaveFileENS_4TextILm0EEES4_NS_4EnumINS_6System11Integration31DocSharingConflictBehavior_EnumEEE");
extern "C" void agiru_unlinked_1372() { Unlinked("agiru::EServices::EDocument::DocumentServiceManagement_Codeunit::SaveFile(agiru::Text<0ul>, agiru::Text<0ul>, agiru::Enum<agiru::System::Integration::DocSharingConflictBehavior_Enum>)"); }
extern "C" void agiru_unlinked_1373() asm("_ZN5agiru9Inventory7Reports31InventorySalesBackOrders_Report10AdoptView_EPKNS_8TableDefEPKv");
extern "C" void agiru_unlinked_1373() { Unlinked("agiru::Inventory::Reports::InventorySalesBackOrders_Report::AdoptView_(agiru::TableDef const*, void const*)"); }
extern "C" void agiru_unlinked_1374() asm("_ZN5agiru9Inventory7Reports31InventorySalesBackOrders_Report11OnPreReportEv");
extern "C" void agiru_unlinked_1374() { Unlinked("agiru::Inventory::Reports::InventorySalesBackOrders_Report::OnPreReport()"); }
extern "C" void agiru_unlinked_1375() asm("_ZN5agiru9Inventory7Reports31InventorySalesBackOrders_Report16OnQueryClosePageENS_6ActionE");
extern "C" void agiru_unlinked_1375() { Unlinked("agiru::Inventory::Reports::InventorySalesBackOrders_Report::OnQueryClosePage(agiru::Action)"); }
extern "C" void agiru_unlinked_1376() asm("_ZN5agiru9Inventory7Reports31InventorySalesBackOrders_Report5Walk_Ev");
extern "C" void agiru_unlinked_1376() { Unlinked("agiru::Inventory::Reports::InventorySalesBackOrders_Report::Walk_()"); }
extern "C" void agiru_unlinked_1377() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page10OnOpenPageEv");
extern "C" void agiru_unlinked_1377() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnOpenPage()"); }
extern "C" void agiru_unlinked_1378() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page11OnActionAllEv");
extern "C" void agiru_unlinked_1378() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnActionAll()"); }
extern "C" void agiru_unlinked_1379() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page12OnEnabledAllEv");
extern "C" void agiru_unlinked_1379() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnEnabledAll()"); }
extern "C" void agiru_unlinked_1380() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page16OnAfterGetRecordEv");
extern "C" void agiru_unlinked_1380() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnAfterGetRecord()"); }
extern "C" void agiru_unlinked_1381() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page17OnActionDayPeriodEv");
extern "C" void agiru_unlinked_1381() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnActionDayPeriod()"); }
extern "C" void agiru_unlinked_1382() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page18OnActionWeekPeriodEv");
extern "C" void agiru_unlinked_1382() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnActionWeekPeriod()"); }
extern "C" void agiru_unlinked_1383() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page18OnActionYearPeriodEv");
extern "C" void agiru_unlinked_1383() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnActionYearPeriod()"); }
extern "C" void agiru_unlinked_1384() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page18OnEnabledDayPeriodEv");
extern "C" void agiru_unlinked_1384() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnEnabledDayPeriod()"); }
extern "C" void agiru_unlinked_1385() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page19OnActionMonthPeriodEv");
extern "C" void agiru_unlinked_1385() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnActionMonthPeriod()"); }
extern "C" void agiru_unlinked_1386() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page19OnEnabledWeekPeriodEv");
extern "C" void agiru_unlinked_1386() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnEnabledWeekPeriod()"); }
extern "C" void agiru_unlinked_1387() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page19OnEnabledYearPeriodEv");
extern "C" void agiru_unlinked_1387() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnEnabledYearPeriod()"); }
extern "C" void agiru_unlinked_1388() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page20OnEnabledMonthPeriodEv");
extern "C" void agiru_unlinked_1388() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnEnabledMonthPeriod()"); }
extern "C" void agiru_unlinked_1389() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page20UpdateChartForVendorENS_4CodeILm20EEE");
extern "C" void agiru_unlinked_1389() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::UpdateChartForVendor(agiru::Code<20ul>)"); }
extern "C" void agiru_unlinked_1390() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page21OnActionQuarterPeriodEv");
extern "C" void agiru_unlinked_1390() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnActionQuarterPeriod()"); }
extern "C" void agiru_unlinked_1391() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page22OnEnabledQuarterPeriodEv");
extern "C" void agiru_unlinked_1391() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnEnabledQuarterPeriod()"); }
extern "C" void agiru_unlinked_1392() asm("_ZN5agiru9Purchases6Vendor24AgedAccPayableChart_Page24OnActionChartInformationEv");
extern "C" void agiru_unlinked_1392() { Unlinked("agiru::Purchases::Vendor::AgedAccPayableChart_Page::OnActionChartInformation()"); }
extern "C" void agiru_unlinked_1393() asm("_ZN5agiru9Purchases7Posting29PurchaseBatchPostMgt_Codeunit12SetParameterENS_4EnumINS_10Foundation15BatchProcessing30BatchPostingParameterType_EnumEEENS_7VariantE");
extern "C" void agiru_unlinked_1393() { Unlinked("agiru::Purchases::Posting::PurchaseBatchPostMgt_Codeunit::SetParameter(agiru::Enum<agiru::Foundation::BatchProcessing::BatchPostingParameterType_Enum>, agiru::Variant)"); }
extern "C" void agiru_unlinked_1394() asm("_ZN5agiru9Purchases7Posting29PurchaseBatchPostMgt_Codeunit17SetBatchProcessorENS_10Foundation15BatchProcessing27BatchProcessingMgt_CodeunitE");
extern "C" void agiru_unlinked_1394() { Unlinked("agiru::Purchases::Posting::PurchaseBatchPostMgt_Codeunit::SetBatchProcessor(agiru::Foundation::BatchProcessing::BatchProcessingMgt_Codeunit)"); }
extern "C" void agiru_unlinked_1395() asm("_ZN5agiru9Purchases7Posting29PurchaseBatchPostMgt_Codeunit8RunBatchERNS0_8Document20PurchaseHeader_TableEbNS_4DateEbbbb");
extern "C" void agiru_unlinked_1395() { Unlinked("agiru::Purchases::Posting::PurchaseBatchPostMgt_Codeunit::RunBatch(agiru::Purchases::Document::PurchaseHeader_Table&, bool, agiru::Date, bool, bool, bool, bool)"); }
extern "C" void agiru_unlinked_1396() asm("_ZN5agiru9Purchases7Posting29PurchaseBatchPostMgt_Codeunit9RunWithUIERNS0_8Document20PurchaseHeader_TableEiNS_4TextILm0EEE");
extern "C" void agiru_unlinked_1396() { Unlinked("agiru::Purchases::Posting::PurchaseBatchPostMgt_Codeunit::RunWithUI(agiru::Purchases::Document::PurchaseHeader_Table&, int, agiru::Text<0ul>)"); }
extern "C" void agiru_unlinked_1397() asm("_ZN5agiru9Utilities35DataClassificationEvalData_Codeunit20CreateEvaluationDataEv");
extern "C" void agiru_unlinked_1397() { Unlinked("agiru::Utilities::DataClassificationEvalData_Codeunit::CreateEvaluationData()"); }
extern "C" void agiru_unlinked_1398() asm("_ZN5agiru9Utilities35DataClassificationEvalData_Codeunit22SetTableFieldsToNormalEi");
extern "C" void agiru_unlinked_1398() { Unlinked("agiru::Utilities::DataClassificationEvalData_Codeunit::SetTableFieldsToNormal(int)"); }
extern "C" void agiru_unlinked_1399() asm("_ZNK5agiru4File6ToTextB5cxx11Ev");
extern "C" void agiru_unlinked_1399() { Unlinked("agiru::File::ToText[abi:cxx11]() const"); }
