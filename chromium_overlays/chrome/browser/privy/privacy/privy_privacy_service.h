#ifndef CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_SERVICE_H_
#define CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_SERVICE_H_

#include <memory>
#include <optional>

#include "base/functional/callback_forward.h"
#include "base/memory/weak_ptr.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/privy_privacy/compute/private_compute_provider.h"
#include "components/privy_privacy/core/privacy_policy_engine.h"

namespace privy {

// Profile-scoped browser service that owns privacy policy decisions and the
// private-compute provider boundary. Site/extension integration code talks to
// this service instead of talking directly to Aleo or another provider.
class PrivyPrivacyService : public KeyedService {
 public:
  using EvaluateCallback = base::OnceCallback<void(
      PrivacyDecision,
      std::optional<PrivateComputeResult>)>;

  PrivyPrivacyService(std::unique_ptr<PrivateComputeProvider> compute_provider,
                      bool off_the_record);
  PrivyPrivacyService(const PrivyPrivacyService&) = delete;
  PrivyPrivacyService& operator=(const PrivyPrivacyService&) = delete;
  ~PrivyPrivacyService() override;

  void EvaluateAndMaybeCompute(
      PrivacyRequest request,
      std::optional<PrivateComputeRequest> compute_request,
      EvaluateCallback callback);

  bool off_the_record() const { return off_the_record_; }
  const PrivateComputeProvider* compute_provider_for_testing() const {
    return compute_provider_.get();
  }

 private:
  void OnPrivateComputeComplete(PrivacyDecision original_decision,
                                EvaluateCallback callback,
                                PrivateComputeResult result);

  PrivacyPolicyEngine policy_engine_;
  std::unique_ptr<PrivateComputeProvider> compute_provider_;
  const bool off_the_record_;

  base::WeakPtrFactory<PrivyPrivacyService> weak_factory_{this};
};

}  // namespace privy

#endif  // CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_SERVICE_H_
