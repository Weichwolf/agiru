#include "runtime/Session.h"
#include "runtime/Table.h"

#include "Check.h"
#include "ResourceCost.h"

#include <cstdlib>
#include <string_view>

using ResourceCost = agiru::Projects::Resources::Pricing::ResourceCost_Table;
using ResourceCostType = agiru::options::OptionResourceGroupResourceAll;

namespace {

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
  if (std::getenv("AGIRU_EXERCISE_STALE_XREC") != nullptr) {
    CHECK_TEXT("the prior reference remains readable", std::string_view(image.Code), "");
  }
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

}

int main() {
  return gate::Run("RecordImage", [] {
    const agiru::Session session(AGIRU_TEST_DSN);
    CopyKeepsALiveXRecReference();
    CopyAndInsertUpdateTheStableImage();
  });
}
