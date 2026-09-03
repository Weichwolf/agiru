#pragma once

#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/ModuleInfo.h"
#include "type/RecordId.h"
#include "type/Stream.h"
#include "type/TextEncoding.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `NavApp` -- the surface the platform documentation declares.

namespace agiru {

class JsonObject;

/// \brief AL `NavApp`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/navapp/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class NavApp {
public:
  /// \brief AL `NavApp.DeleteArchiveData(Integer)`. Deletes the archived data for a specified table
  /// of an extension during installation.
  /// \param TableNo The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void DeleteArchiveData(::agiru::Integer TableNo);

  /// \brief AL `NavApp.GetArchiveRecordRef(Integer, RecordRef)`. Returns a RecordRef for the
  /// specified table.
  /// \param TableNo The AL `Integer`.
  /// \param RecordRef The AL `RecordRef`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean GetArchiveRecordRef(::agiru::Integer TableNo,
                                              ::agiru::RecordRef &RecordRef);

  /// \brief AL `NavApp.GetArchiveVersion()`. Returns the version of the extension that the
  /// specified table is part of.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static std::string GetArchiveVersion();

  /// \brief AL `NavApp.GetCallerCallstackModuleInfos()`. Gets information about extensions on the
  /// callstack that contain the method, which called the currently running method. For example, if
  /// method 1 (in extension A) calls method 2 (in extension B) calls method 3 (in extension C),
  /// which calls GetCallerModuleInfo, then GetCallerModuleInfo will return information about
  /// extension A and B.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void GetCallerCallstackModuleInfos();

  /// \brief AL `NavApp.GetCallerModuleInfo(ModuleInfo)`. Gets information about the extension that
  /// contains the method that called the currently running method. For example, if method 1 (in
  /// extension A) calls method 2 (in extension B), which calls GetCallerModuleInfo, then
  /// GetCallerModuleInfo will return information about extension A.
  /// \param Info The AL `ModuleInfo`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean GetCallerModuleInfo(::agiru::ModuleInfo &Info);

  /// \brief AL `NavApp.GetCallstackModuleInfos()`. Gets application information about each method
  /// in the current callstack.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void GetCallstackModuleInfos();

  /// \brief AL `NavApp.GetCurrentModuleInfo(ModuleInfo)`. Gets information about the application
  /// that contains the AL object that is currently running.
  /// \param Info The AL `ModuleInfo`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean GetCurrentModuleInfo(::agiru::ModuleInfo &Info);

  /// \brief AL `NavApp.GetModuleInfo(Guid, ModuleInfo)`. Gets information about the specified AL
  /// application.
  /// \param AppId The AL `Guid`.
  /// \param Info The AL `ModuleInfo`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean GetModuleInfo(::agiru::Guid AppId, ::agiru::ModuleInfo &Info);

  /// \brief AL `NavApp.GetResource(Text, InStream, TextEncoding)`. Retrieves a resource that was
  /// packaged with this app and loads it into the specified InStream
  /// \param ResourceName The AL `Text`.
  /// \param ResourceStream The AL `InStream`.
  /// \param Encoding The AL `TextEncoding`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void GetResource(std::string_view ResourceName,
                          ::agiru::InStream &ResourceStream,
                          const ::agiru::TextEncoding &Encoding);

  /// \brief AL `NavApp.GetResourceAsJson(Text, TextEncoding)`. Retrieves the specified resource as
  /// a JsonObject
  /// \param ResourceName The AL `Text`.
  /// \param Encoding The AL `TextEncoding`.
  /// \return The AL `JsonObject`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::JsonObject GetResourceAsJson(std::string_view ResourceName,
                                               const ::agiru::TextEncoding &Encoding);

  /// \brief AL `NavApp.GetResourceAsText(Text, TextEncoding)`. Retrieves the specified resource as
  /// Text
  /// \param ResourceName The AL `Text`.
  /// \param Encoding The AL `TextEncoding`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static std::string GetResourceAsText(std::string_view ResourceName,
                                       const ::agiru::TextEncoding &Encoding);

  /// \brief AL `NavApp.IsEntitled(Text, Guid)`. Determines if the current user is entitled to a
  /// specific entitlement id for the application.
  /// \param Id The AL `Text`.
  /// \param AppId The AL `Guid`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean IsEntitled(std::string_view Id, ::agiru::Guid AppId);

  /// \brief AL `NavApp.IsInstalling()`. Returns **true** if the application that contains the AL
  /// object that is currently running is being installed, otherwise it returns **false**.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean IsInstalling();

  /// \brief AL `NavApp.IsUnlicensed(Guid)`. Determines if the current user is assigned the
  /// Unlicensed entitlement type for the application.
  /// \param AppId The AL `Guid`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean IsUnlicensed(::agiru::Guid AppId);

  /// \brief AL `NavApp.ListResources(Text)`. Gets an optionally filtered list of resources packaged
  /// with this app.
  /// \param Filter The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void ListResources(std::string_view Filter);

  /// \brief AL `NavApp.LoadPackageData(Integer)`. Loads default, or starting, table data into the
  /// specified table of an extension during installation.
  /// \param TableNo The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void LoadPackageData(::agiru::Integer TableNo);

  /// \brief AL `NavApp.RestoreArchiveData(Integer, Boolean)`. Restores archived data for a
  /// specified table of an extension during installation.
  /// \param TableNo The AL `Integer`.
  /// \param RunTrigger The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean RestoreArchiveData(::agiru::Integer TableNo, ::agiru::Boolean RunTrigger);
};

}
