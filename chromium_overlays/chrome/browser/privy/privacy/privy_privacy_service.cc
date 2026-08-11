#include "chrome/browser/privy/privacy/privy_privacy_service.h"

#include <utility>

#include "base/functional/bind.h"

namespace privy {

PrivyPrivacyService::PrivyPrivacyService(
    std::unique_ptr<PrivateComputeProvider> compute_provider,
    bool off_the_record)
    : compute_provider_(std::move(compute_provider)),
      off_the_record_(off_the_record) {}

PrivyPrivacyService::~PrivyPrivacyService() = default;

void PrivyPrivacyService::EvaluateAndMaybeCompute(
    PrivacyRequest request,
    std::optional<PrivateComputeRequest> compute_request,
    EvaluateCallback callback) {
  request.supports_private_compute =
      compute_request.has_value() && compute_provider_ &&
      compute_provider_->Supports(compute_request->capability);

  PrivacyDecision decision = policy_engine_.Evaluate(request);
  if (decision.action != PrivacyAction::kComputePrivately) {
    std::move(callback).Run(std::move(decision), std::nullopt);
    return;
  }

  // `supports_private_compute` was derived above from these same conditions,
  // so this is defensive rather than a normal path.
  if (!compute_request || !compute_provider_) {
    std::move(callback).Run(
        PrivacyDecision{PrivacyAction::kBlock,
                        "private_compute_provider_unavailable"},
        std::nullopt);
    return;
  }

  compute_provider_->Execute(
      std::move(*compute_request),
      base::BindOnce(&PrivyPrivacyService::OnPrivateComputeComplete,
                     weak_factory_.GetWeakPtr(), std::move(decision),
                     std::move(callback)));
}

void PrivyPrivacyService::OnPrivateComputeComplete(
    PrivacyDecision original_decision,
    EvaluateCallback callback,
    PrivateComputeResult result) {
  if (!result.success) {
    std::move(callback).Run(
        PrivacyDecision{PrivacyAction::kBlock, "private_compute_failed"},
        std::move(result));
    return;
  }

  std::move(callback).Run(std::move(original_decision), std::move(result));
}

}  // namespace privy
