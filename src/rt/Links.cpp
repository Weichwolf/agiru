#include "platform/RecordLink.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Table.h"
#include "type/DateTime.h"
#include "type/RecordId.h"
#include "type/Variant.h"

#include <string_view>

namespace agiru::detail {

Integer RuntimeAddLink(const RecordId &to, std::string_view url, std::string_view description) {
  platform::RecordLink link;
  link.Init();
  link.RecordID = to;
  link.URL1 = url;
  link.Description = description;
  link.Type = platform::RecordLinkType::Link;
  link.Created = CurrentDateTime();
  link.Company = Session::Current().CompanyName();
  link.Insert();
  return link.LinkID;
}

void RuntimeCopyLinks(const RecordId &from, const RecordId &to) {
  platform::RecordLink source;
  source.SetRange(source.RecordID, from);
  if (!source.FindSet()) { return; }
  do {
    platform::RecordLink copy;
    copy.Init();
    copy.TransferFields(source, false);
    copy.LinkID = 0;
    copy.RecordID = to;
    copy.Insert();
  } while (source.Next() != 0);
}

void RuntimeDeleteLinks(const RecordId &of) {
  platform::RecordLink link;
  link.SetRange(link.RecordID, of);
  link.DeleteAll();
}

bool RuntimeHasLinks(const RecordId &of) {
  platform::RecordLink link;
  link.SetRange(link.RecordID, of);
  return !link.IsEmpty();
}

RecordId RecordIdInVariant(const Variant &held) {
  if (held.IsRecord()) { return held.Get<RecordInVariant>().id; }
  if (held.IsRecordRef()) { return static_cast<const RecordRef &>(held).RecordId(); }
  if (held.IsRecordId()) { return held.Get<RecordId>(); }
  throw Error("CopyLinks needs a record, a RecordRef or a RecordId, and this Variant holds none");
}

}
