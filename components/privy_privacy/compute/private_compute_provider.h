#ifndef COMPONENTS_PRIVY_PRIVACY_COMPUTE_PRIVATE_COMPUTE_PROVIDER_H_
#define COMPONENTS_PRIVY_PRIVACY_COMPUTE_PRIVATE_COMPUTE_PROVIDER_H_

#include "base/functional/callback_forward.h"
#include "components/privy_privacy/compute/private_compute_types.h"

namespace privy {

class PrivateComputeProvider {
 public:
  using ExecuteCallback = base::OnceCallback<void(PrivateComputeResult)>;

  virtual ~PrivateComputeProvider() = default;

  virtual bool Supports(PrivateComputeCapability capability) const = 0;
  virtual void Execute(PrivateComputeRequest request,
                       ExecuteCallback callback) = 0;
};

}  // namespace privy

#endif  // COMPONENTS_PRIVY_PRIVACY_COMPUTE_PRIVATE_COMPUTE_PROVIDER_H_
