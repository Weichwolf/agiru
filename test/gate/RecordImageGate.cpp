#include "runtime/Database.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/Transaction.h"
#include "type/Decimal.h"

#include "Check.h"
#include "LineNumberBuffer.h"
#include "ResourceCost.h"
#include "options/Types.h"

#include <new>
#include <string_view>

using ResourceCost = agiru::Projects::Resources::Pricing::ResourceCost_Table;
using ResourceCostType = agiru::options::OptionResourceGroupResourceAll;

namespace {

class FaultImage {
public:
  FaultImage() { ++live; }

  FaultImage(const FaultImage &other) : value(other.value) {
    if (failCopy) { throw std::bad_alloc(); }
    ++live;
  }

  FaultImage(FaultImage &&other) noexcept : value(other.value) { ++live; }

  FaultImage &operator=(const FaultImage &other) {
    if (this == &other) { return *this; }
    if (failAssign) { throw std::bad_alloc(); }
    value = other.value;
    return *this;
  }

  FaultImage &operator=(FaultImage &&other) noexcept {
    value = other.value;
    return *this;
  }

  ~FaultImage() { --live; }

  static inline int live = 0;
  static inline bool failCopy = false;
  static inline bool failAssign = false;
  int value = 0;
};

void AFailedImageUpdateReleasesTheIncomingRecord() {
  agiru::detail::HeldImage image;
  image.Hold(new FaultImage());
  void *const address = image.Get();
  FaultImage::failAssign = true;
  bool refused = false;
  try {
    image.Hold(new FaultImage());
  } catch (const std::bad_alloc &) { refused = true; }
  FaultImage::failAssign = false;
  CHECK_TRUE("the image assignment exercised its allocation failure", refused);
  CHECK_TRUE("failed image assignment retains the live owner", image.Get() == address);
  CHECK_TRUE("failed image assignment frees the incoming record", FaultImage::live == 1);
}

void AFailedCloneRetainsThePreviousImage() {
  agiru::detail::HeldImage source;
  source.Hold(new FaultImage());
  agiru::detail::HeldImage destination;
  destination.Hold(new int(1));
  void *const address = destination.Get();
  FaultImage::failCopy = true;
  bool refused = false;
  try {
    destination.SetFrom(source);
  } catch (const std::bad_alloc &) { refused = true; }
  FaultImage::failCopy = false;
  CHECK_TRUE("the replacement clone exercised its allocation failure", refused);
  CHECK_TRUE("failed cloning leaves the previous image owned", destination.Get() == address);
}

void AFailedStateCopyKeepsItsLiveImageOwner() {
  agiru::detail::StateHandle source;
  source.Ensure().image.Hold(new FaultImage());
  source.Ensure().group = 2;
  agiru::detail::StateHandle destination;
  destination.Ensure().image.Hold(new FaultImage());
  destination.Ensure().group = 1;
  void *const address = destination.Ensure().image.Get();
  FaultImage::failAssign = true;
  bool refused = false;
  try {
    destination.CopyStateFrom(source);
  } catch (const std::bad_alloc &) { refused = true; }
  FaultImage::failAssign = false;
  CHECK_TRUE("Copy exercised its image assignment failure", refused);
  CHECK_TRUE("failed Copy retains the original live image owner",
             destination.Ensure().image.Get() == address);
  CHECK_TRUE("failed Copy keeps the previous filter state", destination.Ensure().group == 1);
}

void FreshImagesAreValueInitialized() {
  agiru::app::tables::LineNumberBuffer row;
  row.OldLineNumber = 10;
  row.NewLineNumber = 100;
  const auto &blank = row.StoredImage();
  CHECK_TRUE("fresh xRec initializes every scalar independently of the buffer",
             blank.OldLineNumber == 0 && blank.NewLineNumber == 0);
  CHECK_TRUE("fresh xRec retains its owned address", &blank == &row.StoredImage());

  ResourceCost cost;
  cost.Code = "UNWRITTEN";
  cost.UnitCost = agiru::Decimal{10};
  const ResourceCost &image = cost.StoredImage();
  CHECK_TEXT("fresh xRec initializes owned text", std::string_view(image.Code), "");
  CHECK_TRUE("fresh xRec initializes exact amounts", image.UnitCost == agiru::Decimal{});
  CHECK_TRUE("fresh xRec initializes option ordinals", image.Type == ResourceCostType::Resource);
  cost.Init();
  CHECK_TRUE("Init keeps the blank image address", &image == &cost.StoredImage());
  CHECK_TRUE("Init leaves the exact amount initialized", image.UnitCost == agiru::Decimal{});
}

void CopyKeepsALiveXRecReference() {
  ResourceCost destination;
  destination.Init();
  ResourceCost &image = destination.StoredImage();
  ResourceCost *const address = &image;

  ResourceCost source;
  source.Init();
  destination.Copy(source);

  CHECK_TRUE("Copy retains the image owner's address while xRec is live",
             &destination.StoredImage() == address);
  CHECK_TEXT("the prior reference remains readable", std::string_view(image.Code), "");
}

void CopyAndInsertUpdateTheStableImage() {
  agiru::Temporary<ResourceCost> source;
  source.Init();
  ResourceCost &sourceImage = source.StoredImage();
  ResourceCost *const sourceAddress = &sourceImage;
  source.Type = ResourceCostType::Resource;
  source.Code = "SOURCE";
  source.Insert();
  CHECK_TRUE("Insert updates the existing image owner", &source.StoredImage() == sourceAddress);
  CHECK_TEXT("the inserted image contains the inserted fields",
             std::string_view(sourceImage.Code),
             "SOURCE");

  ResourceCost destination;
  destination.Init();
  ResourceCost &destinationImage = destination.StoredImage();
  ResourceCost *const destinationAddress = &destinationImage;
  destination.Copy(source);
  CHECK_TRUE("Copy retains the destination image owner",
             &destination.StoredImage() == destinationAddress);
  CHECK_TEXT("Copy updates the destination image from the source",
             std::string_view(destinationImage.Code),
             "SOURCE");
}

template <typename Record> void GetAndModifyKeepTheLiveImage() {
  // [SET] Distinct LCY fixture amounts distinguish stored, modified and other-row values.
  constexpr int kInsertedLCY = 10;
  constexpr int kModifiedLCY = 20;
  constexpr int kOtherRowLCY = 30;
  constexpr int kFinalLCY = 40;
  Record record;
  record.Init();
  ResourceCost &image = record.StoredImage();
  ResourceCost *const address = &image;
  record.Type = ResourceCostType::Resource;
  record.Code = "FIRST";
  record.UnitCost = agiru::Decimal{kInsertedLCY};
  record.Insert();
  record.UnitCost = agiru::Decimal{kModifiedLCY};
  record.Modify();
  CHECK_TRUE("Modify preserves the image owner", &record.StoredImage() == address);
  CHECK_TRUE("the held image reflects the completed Modify",
             image.UnitCost == agiru::Decimal{kModifiedLCY});

  record.Init();
  record.Code = "SECOND";
  record.UnitCost = agiru::Decimal{kOtherRowLCY};
  record.Insert();
  record.Get(ResourceCostType::Resource, "FIRST");
  CHECK_TRUE("Get preserves the image owner across two record identities",
             &record.StoredImage() == address);
  CHECK_TEXT(
      "Get replaces the image's previous record identity", std::string_view(image.Code), "FIRST");
  CHECK_TRUE("Get captures the stored value rather than the abandoned buffer",
             image.UnitCost == agiru::Decimal{kModifiedLCY});
  record.UnitCost = agiru::Decimal{kFinalLCY};
  CHECK_TRUE("editing the buffer does not change its stored image",
             image.UnitCost == agiru::Decimal{kModifiedLCY});
  record.Modify();
  CHECK_TRUE("a subsequent Modify updates the same held image",
             image.UnitCost == agiru::Decimal{kFinalLCY});
}

void SQLReadsAndWritesKeepTheImage() {
  const agiru::detail::Scope isolation;
  const agiru::Connection &database = agiru::Session::Current().Database();
  agiru::DropTable(database, agiru::TableTraits<ResourceCost>::kTable);
  agiru::CreateTable(database, agiru::TableTraits<ResourceCost>::kTable);
  GetAndModifyKeepTheLiveImage<ResourceCost>();
}

}

int main() {
  return gate::Run("RecordImage", [] {
    AFailedImageUpdateReleasesTheIncomingRecord();
    AFailedCloneRetainsThePreviousImage();
    AFailedStateCopyKeepsItsLiveImageOwner();
    const agiru::Session session(AGIRU_TEST_DSN);
    FreshImagesAreValueInitialized();
    CopyKeepsALiveXRecReference();
    CopyAndInsertUpdateTheStableImage();
    GetAndModifyKeepTheLiveImage<agiru::Temporary<ResourceCost>>();
    SQLReadsAndWritesKeepTheImage();
  });
}
