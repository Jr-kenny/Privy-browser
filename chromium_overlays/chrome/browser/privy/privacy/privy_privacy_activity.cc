#include "chrome/browser/privy/privacy/privy_privacy_activity.h"

namespace privy {

const char* PrivacyActivityPrivateInputTypeToString(
    PrivacyActivityPrivateInputType type) {
  switch (type) {
    case PrivacyActivityPrivateInputType::kNone:
      return "none";
    case PrivacyActivityPrivateInputType::kFrequencyCap:
      return "frequency_cap";
    case PrivacyActivityPrivateInputType::kConversionAttribution:
      return "conversion_attribution";
    case PrivacyActivityPrivateInputType::kTelemetryBucket:
      return "telemetry_bucket";
    case PrivacyActivityPrivateInputType::kPersonalizationMatch:
      return "personalization_match";
  }

  return "unknown";
}

const char* PrivacyActivityDisclosureTypeToString(
    PrivacyActivityDisclosureType type) {
  switch (type) {
    case PrivacyActivityDisclosureType::kNone:
      return "none";
    case PrivacyActivityDisclosureType::kBooleanResult:
      return "boolean_result";
    case PrivacyActivityDisclosureType::kCoarseBucket:
      return "coarse_bucket";
  }

  return "unknown";
}

}  // namespace privy
