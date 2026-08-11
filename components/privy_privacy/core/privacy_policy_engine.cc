#include "components/privy_privacy/core/privacy_policy_engine.h"

namespace privy {

PrivacyDecision PrivacyPolicyEngine::Evaluate(
    const PrivacyRequest& request) const {
  if (request.requester_type == RequesterType::kBrowser) {
    return {PrivacyAction::kAllow, "browser_internal"};
  }

  if (request.surface == PrivacySurface::kFingerprinting) {
    return {PrivacyAction::kSanitize, "reduce_fingerprinting_surface"};
  }

  if (request.surface == PrivacySurface::kExtensionEgress &&
      request.contains_behavioral_data) {
    return {PrivacyAction::kPrompt, "extension_behavioral_data_egress"};
  }

  if ((request.surface == PrivacySurface::kCookie ||
       request.surface == PrivacySurface::kStorage) &&
      request.cross_site && request.persistent) {
    return {PrivacyAction::kBlock, "cross_site_persistent_state"};
  }

  const bool privately_computable_surface =
      request.surface == PrivacySurface::kTelemetry ||
      request.surface == PrivacySurface::kAttribution ||
      request.surface == PrivacySurface::kFrequencyCap ||
      request.surface == PrivacySurface::kPersonalization;

  if (privately_computable_surface && request.contains_behavioral_data) {
    if (request.supports_private_compute) {
      return {PrivacyAction::kComputePrivately,
              "behavioral_data_has_private_compute_path"};
    }

    // Fail closed. A compute-provider outage must never silently downgrade into
    // sending raw behavioral inputs to the requester.
    return {PrivacyAction::kBlock,
            "behavioral_data_private_compute_unavailable"};
  }

  return {PrivacyAction::kAllow, "no_privacy_policy_override"};
}

}  // namespace privy
