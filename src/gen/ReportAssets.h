#pragma once

#include "Ast.h"
#include "NativeSource.h"

#include <string>
#include <vector>

namespace agiru::gen {

struct ReportAssetApp {
  NativeAppIdentity identity;
  std::string source;
  bool platform = false;
};

struct ReportAssetRequest {
  int report = 0;
  std::string target;
  al::ReportLayoutDecl layout;
};

struct ReportAssetRequests {
  std::vector<ReportAssetApp> apps;
  std::vector<ReportAssetRequest> layouts;
};

void AddReportAssets(ReportAssetRequests &requests, const al::PageObject &report);

std::string ReportAssetsManifest(const ReportAssetRequests &requests);

}
