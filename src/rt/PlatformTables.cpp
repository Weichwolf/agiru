#include "platform/AllObj.h"
#include "platform/AllObjWithCaption.h"
#include "platform/AllProfile.h"
#include "platform/Company.h"
#include "platform/Date.h"
#include "platform/Field.h"
#include "platform/Integer.h"
#include "platform/User.h"
#include "platform/UserPersonalization.h"
#include "runtime/Catalogue.h"

namespace agiru {
namespace {

const RegisterTable<platform::AllObj> kAllObj;
const RegisterTable<platform::AllObjWithCaption> kAllObjWithCaption;
const RegisterTable<platform::AllProfile> kAllProfile;
const RegisterTable<platform::Company> kCompany;
const RegisterTable<platform::Date> kDate;
const RegisterTable<platform::Field> kField;
const RegisterTable<platform::Integer> kInteger;
const RegisterTable<platform::User> kUser;
const RegisterTable<platform::UserPersonalization> kUserPersonalization;

}
}
