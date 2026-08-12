#ifndef COMPONENTS_PRIVY_PRIVACY_CORE_PRIVACY_TYPES_H_
#define COMPONENTS_PRIVY_PRIVACY_CORE_PRIVACY_TYPES_H_

#include <string>

#include "url/origin.h"

namespace privy {

enum class RequesterType {
  kSite,
  kExtension,
  kBrowser,
};

enum class PrivacySurface {
  kTelemetry,
  kAttribution,
  kFrequencyCap,
  kPersonalization,
  kCookie,
  kStorage,
  kExtensionEgress,
  kFingerprinting,
};

enum class PrivacyAction {
  kAllow,
  kBlock,
  kSanitize,
  kPrompt,
  kComputePrivately,
};

struct PrivacyRequest {
  url::Origin requester;
  RequesterType requester_type = RequesterType::kSite;
  PrivacySurface surface = PrivacySurface::kTelemetry;

  // True when the request can link state or observations across top-level
  // sites/origins rather than remaining scoped to the current first party.
  bool cross_site = false;

  // True when the request intends to create or access durable state.
  bool persistent = false;

  // True when fulfilling the request with raw inputs would disclose browsing
  // behavior, interaction history, interest/profile data, or similar signals.
  bool contains_behavioral_data = false;

  // True when the browser has a registered private-compute capability that can
  // answer the request without releasing the underlying sensitive inputs.
  bool supports_private_compute = false;

  // Some decisions may be relaxed when the user deliberately initiated an
  // action. A user gesture never bypasses hard security boundaries.
  bool user_gesture = false;
};

struct PrivacyDecision {
  PrivacyAction action = PrivacyAction::kAllow;
  std::string reason;
};

const char* PrivacyActionToString(PrivacyAction action);
const char* PrivacySurfaceToString(PrivacySurface surface);
const char* RequesterTypeToString(RequesterType type);

}  // namespace privy

#endif  // COMPONENTS_PRIVY_PRIVACY_CORE_PRIVACY_TYPES_H_
