#pragma once

#include "dotnet/Refused.h"
#include "type/Boolean.h"
#include "type/Integer.h"

/// \file
/// \brief .NET `Microsoft.Dynamics.Nav.Client.Capabilities.CameraProvider` and the options it
///        takes, rebuilt: a service tier has no camera, so the capability answers that it is not
///        available rather than refusing the question.

namespace agiru::dotnet {

/// \brief .NET `CameraOptions`: what a picture request asks for.
class CameraOptions {
public:
  /// \brief The binder behind `O := O.CameraOptions()`.
  struct Binder {
    /// \brief `new CameraOptions()`. \return The options.
    [[nodiscard]] class CameraOptions operator()() const { return {}; }
  };

  /// \brief The constructor AL calls as a member.
  Binder CameraOptions;

  /// \brief `Quality`. \return The quality, 0 here.
  [[nodiscard]] Integer Quality() const { return quality_; }

  /// \brief `Quality := Integer`. \param quality The quality.
  void Quality(Integer quality) { quality_ = quality; }

  /// \brief `EncodingType`, refused: the client decides it and there is no client.
  ::agiru::dotnet::Refused EncodingType{{.type = "CameraOptions", .member = "EncodingType"}};

  /// \brief `AllowEdit`, refused for the same reason.
  ::agiru::dotnet::Refused AllowEdit{{.type = "CameraOptions", .member = "AllowEdit"}};

  /// \brief `SourceType`, which `Media Upload` sets before it asks.
  ::agiru::dotnet::Refused SourceType{{.type = "CameraOptions", .member = "SourceType"}};

  /// \brief `MediaType`, set the same way.
  ::agiru::dotnet::Refused MediaType{{.type = "CameraOptions", .member = "MediaType"}};

private:
  Integer quality_ = 0;
};

/// \brief .NET `CameraProvider`: the client capability the `Camera` and `Media Upload` pages ask
///        for before they open.
class CameraProvider {
public:
  /// \brief The binder behind `P := P.CameraProvider()`.
  struct Binder {
    /// \brief `new CameraProvider()`. \return The provider.
    [[nodiscard]] class CameraProvider operator()() const { return {}; }
  };

  /// \brief The constructor AL calls as a member.
  Binder CameraProvider;

  /// \brief `CameraProvider.Create()`, the static the BaseApp calls on the variable.
  /// \return A provider.
  [[nodiscard]] static class CameraProvider Create() { return {}; }

  /// \brief `IsAvailable()`.
  /// \return `false`: this tier has no client and therefore no camera.
  /// \note IT ANSWERS RATHER THAN REFUSING, because the BaseApp asks exactly this question to
  ///       decide whether to show the camera at all, and "no camera" is the true answer here.
  [[nodiscard]] Boolean IsAvailable() const { return false; }

  /// \brief `RequestPictureAsync(options)`, refused: nothing can take the picture.
  ::agiru::dotnet::Refused RequestPictureAsync{
      {.type = "CameraProvider", .member = "RequestPictureAsync"}};

  /// \brief `PictureAvailable`, the event the client would raise.
  ::agiru::dotnet::Refused PictureAvailable{
      {.type = "CameraProvider", .member = "PictureAvailable"}};
};

}
