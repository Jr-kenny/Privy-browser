#ifndef COMPONENTS_PRIVY_PRIVACY_COMPUTE_DETERMINISTIC_PRIVATE_COMPUTE_PROVIDER_H_
#define COMPONENTS_PRIVY_PRIVACY_COMPUTE_DETERMINISTIC_PRIVATE_COMPUTE_PROVIDER_H_

#include "components/privy_privacy/compute/private_compute_provider.h"

namespace privy {

// A small provider used to validate the browser architecture without coupling
// integration tests to Aleo/WASM. It intentionally implements the exact same
// typed provider contract as the production Aleo provider.
class DeterministicPrivateComputeProvider final : public PrivateComputeProvider {
 public:
  DeterministicPrivateComputeProvider() = default;
  DeterministicPrivateComputeProvider(
      const DeterministicPrivateComputeProvider&) = delete;
  DeterministicPrivateComputeProvider& operator=(
      const DeterministicPrivateComputeProvider&) = delete;
  ~DeterministicPrivateComputeProvider() override = default;

  bool Supports(PrivateComputeCapability capability) const override;
  void Execute(PrivateComputeRequest request,
               ExecuteCallback callback) override;
};

}  // namespace privy

#endif  // COMPONENTS_PRIVY_PRIVACY_COMPUTE_DETERMINISTIC_PRIVATE_COMPUTE_PROVIDER_H_
