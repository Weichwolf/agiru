#pragma once

#include "runtime/Database.h"

#include <array>
#include <string_view>

namespace gate {

inline void InstallNativePermissionFixture(const agiru::Connection &connection) {
  constexpr std::array<std::string_view, 4> statements{
      R"(CREATE TABLE "Access Control" (
 "User Security ID" uuid NOT NULL, "Role ID" varchar(20) NOT NULL,
 "Company Name" varchar(30) NOT NULL, "Scope" integer NOT NULL, "App ID" uuid NOT NULL,
 PRIMARY KEY("User Security ID","Role ID","Company Name","Scope","App ID")))",
      R"(CREATE TABLE "Tenant Permission Set" (
 "App ID" uuid NOT NULL, "Role ID" varchar(20) NOT NULL, "Name" varchar(30) NOT NULL,
 "Assignable" boolean NOT NULL, PRIMARY KEY("App ID","Role ID")))",
      R"(CREATE TABLE "Tenant Permission" (
 "App ID" uuid NOT NULL, "Role ID" varchar(20) NOT NULL, "Object Type" integer NOT NULL,
 "Object ID" integer NOT NULL, "Read Permission" integer NOT NULL,
 "Insert Permission" integer NOT NULL, "Modify Permission" integer NOT NULL,
 "Delete Permission" integer NOT NULL, "Execute Permission" integer NOT NULL,
 "Security Filter" text NOT NULL, "Type" integer NOT NULL,
 PRIMARY KEY("App ID","Role ID","Object Type","Object ID")))",
      R"(CREATE TABLE "Tenant Permission Set Rel." (
 "App ID" uuid NOT NULL, "Role ID" varchar(30) NOT NULL, "Related App ID" uuid NOT NULL,
 "Related Role ID" varchar(30) NOT NULL, "Type" integer NOT NULL,
 "Related Scope" integer NOT NULL,
 PRIMARY KEY("App ID","Role ID","Related App ID","Related Role ID")))"};
  for (const auto statement : statements) { connection.Run(statement); }
}

}
