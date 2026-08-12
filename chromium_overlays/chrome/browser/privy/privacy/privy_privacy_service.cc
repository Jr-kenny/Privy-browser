#include "chrome/browser/privy/privacy/privy_privacy_service.h"

#include <utility>
#include <variant>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/time/time.h"

namespace privy {
namespace {

PrivacyActivityPrivateInputType ActivityInputTypeForCapability(
    PrivateComputeCapability capability) {
  switch (capability) {
    case PrivateComputeCapability::kFrequencyCap:
      return PrivacyActivityPrivateInputType::kFrequencyCap;
    case PrivateComputeCapability::kConversionAttribution:
      return PrivacyActivityPrivateInputType::kConversionAttribution;
    case PrivateComputeCapability::kTelemetryBucket:
      return PrivacyActivityPrivateInputType::kTelemetryBucket;
    case PrivateComputeCapability::kPersonalizationMatch:
      return PrivacyActivityPrivateInputType::kPersonalizationMatch;
  }

  return PrivacyActivityPrivateInputType::kNone;
}

uint32_t ActivityInputCountForCapability(PrivateComputeCapability capability) {
  switch (capability) {
    case PrivateComputeCapability::kFrequencyCap:
      return 3;
    case PrivateComputeCapability::kConversionAttribution:
      return 3;
    case PrivateComputeCapability::kTelemetryBucket:
      return 2;
    case PrivateComputeCapability::kPersonalizationMatch:
      return 2;
  }

  return 0;
}

}  // namespace

PrivyPrivacyService::PrivyPrivacyService(
    std::unique_ptr<PrivateComputeProvider> compute_provider,
    bool off_the_record)
    : compute_provider_(std::move(compute_provider)),
      off_the_record_(off_the_record) {}

PrivyPrivacyService::~PrivyPrivacyService() = default;

void PrivyPrivacyService::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void PrivyPrivacyService::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void PrivyPrivacyService::ClearActivity() {
  activity_.clear();
  for (Observer& observer : observers_) {
    observer.OnPrivacyActivityCleared();
  }
}

size_t PrivyPrivacyService::GetActivityCount(PrivacyAction action) const {
  size_t count = 0;
  for (const PrivacyActivity& activity : activity_) {
    if (activity.decision == action) {
      ++count;
    }
  }
  return count;
}

void PrivyPrivacyService::EvaluateAndMaybeCompute(
    PrivacyRequest request,
    std::optional<PrivateComputeRequest> compute_request,
    EvaluateCallback callback) {
  request.supports_private_compute =
      compute_request.has_value() && compute_provider_ &&
      compute_provider_->Supports(compute_request->capability);

  PrivacyDecision decision = policy_engine_.Evaluate(request);
  if (decision.action != PrivacyAction::kComputePrivately) {
    RecordActivity(request, decision.action, decision.reason, std::nullopt,
                   nullptr);
    std::move(callback).Run(std::move(decision), std::nullopt);
    return;
  }

  // `supports_private_compute` was derived above from these same conditions,
  // so this is defensive rather than a normal path.
  if (!compute_request || !compute_provider_) {
    PrivacyDecision failed_decision{
        PrivacyAction::kBlock, "private_compute_provider_unavailable"};
    RecordActivity(request, failed_decision.action, failed_decision.reason,
                   std::nullopt, nullptr);
    std::move(callback).Run(std::move(failed_decision), std::nullopt);
    return;
  }

  const PrivateComputeCapability capability = compute_request->capability;
  compute_provider_->Execute(
      std::move(*compute_request),
      base::BindOnce(&PrivyPrivacyService::OnPrivateComputeComplete,
                     weak_factory_.GetWeakPtr(), std::move(request),
                     std::move(decision), capability, std::move(callback)));
}

void PrivyPrivacyService::OnPrivateComputeComplete(
    PrivacyRequest request,
    PrivacyDecision original_decision,
    PrivateComputeCapability capability,
    EvaluateCallback callback,
    PrivateComputeResult result) {
  if (!result.success) {
    PrivacyDecision failed_decision{PrivacyAction::kBlock,
                                    "private_compute_failed"};
    RecordActivity(request, failed_decision.action, failed_decision.reason,
                   capability, &result);
    std::move(callback).Run(std::move(failed_decision), std::move(result));
    return;
  }

  RecordActivity(request, original_decision.action, original_decision.reason,
                 capability, &result);
  std::move(callback).Run(std::move(original_decision), std::move(result));
}

void PrivyPrivacyService::RecordActivity(
    const PrivacyRequest& request,
    PrivacyAction decision,
    const std::string& reason,
    std::optional<PrivateComputeCapability> capability,
    const PrivateComputeResult* result) {
  PrivacyActivity activity;
  activity.timestamp = base::Time::Now();
  activity.requester = request.requester;
  activity.requester_type = request.requester_type;
  activity.surface = request.surface;
  activity.decision = decision;
  activity.reason = reason;

  if (capability.has_value()) {
    activity.private_compute_attempted = true;
    activity.private_input_type =
        ActivityInputTypeForCapability(*capability);
    activity.private_input_count = ActivityInputCountForCapability(*capability);

    if (result && result->success) {
      activity.private_compute_succeeded = true;
      if (std::holds_alternative<bool>(result->value)) {
        activity.disclosed_type =
            PrivacyActivityDisclosureType::kBooleanResult;
        activity.disclosed_field_count = 1;
      } else if (std::holds_alternative<uint32_t>(result->value)) {
        activity.disclosed_type =
            PrivacyActivityDisclosureType::kCoarseBucket;
        activity.disclosed_field_count = 1;
      }
    }
  }

  if (activity_.size() == kMaxActivityEntries) {
    activity_.pop_front();
  }
  activity_.push_back(activity);

  for (Observer& observer : observers_) {
    observer.OnPrivacyActivityAdded(activity);
  }
}

}  // namespace privy
