#include "components/privy_privacy/core/privacy_types.h"

namespace privy {

const char* PrivacyActionToString(PrivacyAction action) {
  switch (action) {
    case PrivacyAction::kAllow:
      return "allow";
    case PrivacyAction::kBlock:
      return "block";
    case PrivacyAction::kSanitize:
      return "sanitize";
    case PrivacyAction::kPrompt:
      return "prompt";
    case PrivacyAction::kComputePrivately:
      return "compute_privately";
  }

  return "unknown";
}

const char* PrivacySurfaceToString(PrivacySurface surface) {
  switch (surface) {
    case PrivacySurface::kTelemetry:
      return "telemetry";
    case PrivacySurface::kAttribution:
      return "attribution";
    case PrivacySurface::kFrequencyCap:
      return "frequency_cap";
    case PrivacySurface::kPersonalization:
      return "personalization";
    case PrivacySurface::kCookie:
      return "cookie";
    case PrivacySurface::kStorage:
      return "storage";
    case PrivacySurface::kExtensionEgress:
      return "extension_egress";
    case PrivacySurface::kFingerprinting:
      return "fingerprinting";
  }

  return "unknown";
}

const char* RequesterTypeToString(RequesterType type) {
  switch (type) {
    case RequesterType::kSite:
      return "site";
    case RequesterType::kExtension:
      return "extension";
    case RequesterType::kBrowser:
      return "browser";
  }

  return "unknown";
}

}  // namespace privy
