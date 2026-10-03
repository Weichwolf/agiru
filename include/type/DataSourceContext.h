#pragma once

#include "meta/Ids.h"
#include "type/Guid.h"
#include "type/Integer.h"

/// \file
/// \brief Runtime-18 metadata passed to an ITestDataSource provider.
namespace agiru {

class DataSourceContext;

namespace detail {
/// \brief Creates provider metadata from the test app and the provider's own declaration.
/// \param app The declaring test app, not the provider app.
/// \param provider The codeunit that supplies data, not the consuming test codeunit.
/// \return An owned immutable context; no ambient session or process state is consulted.
/// \throws Error If provider is not a positive AL codeunit ID.
[[nodiscard]] DataSourceContext makeDataSourceContext(Guid app, ::agiru::CodeunitId provider);
}

/// \brief AL DataSourceContext; copies retain the same immutable app/provider identity.
class DataSourceContext {
public:
  /// \brief An unbound variable; getters refuse until a provider context is assigned.
  /// \note Unbound-variable behaviour is conservative, not a measured BC default-value claim.
  DataSourceContext() = default;

  /// \return The GUID of the app that declares the test function.
  /// \throws Error If this variable has no provider context.
  [[nodiscard]] Guid AppId() const;

  /// \return The ID of the data-provider codeunit.
  /// \throws Error If this variable has no provider context.
  [[nodiscard]] Integer CodeunitId() const;

private:
  friend DataSourceContext detail::makeDataSourceContext(Guid app, ::agiru::CodeunitId provider);

  DataSourceContext(Guid app, ::agiru::CodeunitId provider) : app_(app), provider_(provider) {}

  Guid app_;
  ::agiru::CodeunitId provider_;
};

}
