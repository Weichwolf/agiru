#include "platform/Date.h"
#include "platform/Field.h"
#include "platform/Integer.h"
#include "platform/User.h"
#include "platform/UserPersonalization.h"
#include "runtime/Catalogue.h"

namespace agiru {
namespace {

const RegisterTable<platform::Date> kDate;
const RegisterTable<platform::Field> kField;
const RegisterTable<platform::Integer> kInteger;
const RegisterTable<platform::User> kUser;
const RegisterTable<platform::UserPersonalization> kUserPersonalization;

}
}
