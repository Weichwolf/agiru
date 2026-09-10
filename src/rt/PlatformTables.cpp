#include "platform/AllObj.h"
#include "platform/AllObjWithCaption.h"
#include "platform/AllProfile.h"
#include "platform/Company.h"
#include "platform/Date.h"
#include "platform/FeatureKey.h"
#include "platform/Field.h"
#include "platform/Integer.h"
#include "platform/PrivacyNotice.h"
#include "platform/PrivacyNoticeApproval.h"
#include "platform/RecordLink.h"
#include "platform/TableMetadata.h"
#include "platform/TenantLicenseState.h"
#include "platform/User.h"
#include "platform/UserPersonalization.h"
#include "runtime/Catalogue.h"

namespace agiru {
namespace {

const RegisterTable<platform::AllObj> kAllObj;
const RegisterTable<platform::AllObjWithCaption> kAllObjWithCaption;
const RegisterTable<platform::AllProfile> kAllProfile;
const RegisterTable<platform::FeatureKey> kFeatureKey;
const RegisterTable<platform::Company> kCompany;
const RegisterTable<platform::Date> kDate;
const RegisterTable<platform::Field> kField;
const RegisterTable<platform::Integer> kInteger;
const RegisterTable<platform::PrivacyNotice> kPrivacyNotice;
const RegisterTable<platform::PrivacyNoticeApproval> kPrivacyNoticeApproval;
const RegisterTable<platform::RecordLink> kRecordLink;
const RegisterTable<platform::TableMetadata> kTableMetadata;
const RegisterTable<platform::TenantLicenseState> kTenantLicenseState;
const RegisterTable<platform::User> kUser;
const RegisterTable<platform::UserPersonalization> kUserPersonalization;

}
}
