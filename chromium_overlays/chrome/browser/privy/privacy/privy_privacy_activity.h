#ifndef CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_ACTIVITY_H_
#define CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_ACTIVITY_H_

#include <cstdint>
#include <string>

#include "base/time/time.h"
#include "components/privy_privacy/core/privacy_types.h"
#include "url/origin.h"

namespace privy {

// These enums intentionally describe categories rather than values. Activity
// records must remain useful to the browser UI without retaining private
// inputs or result details.
enum class PrivacyActivityPrivateInputType {
  kNone,
  kFrequencyCap,
  kConversionAttribution,
  kTelemetryBucket,
  kPersonalizationMatch,
};

enum class PrivacyActivityDisclosureType {
  kNone,
  kBooleanResult,
  kCoarseBucket,
};

const char* PrivacyActivityPrivateInputTypeToString(
    PrivacyActivityPrivateInputType type);
const char* PrivacyActivityDisclosureTypeToString(
    PrivacyActivityDisclosureType type);

struct PrivacyActivity {
  base::Time timestamp;
  url::Origin requester;
  RequesterType requester_type = RequesterType::kSite;
  PrivacySurface surface = PrivacySurface::kTelemetry;
  PrivacyAction decision = PrivacyAction::kAllow;

  // Only category and count metadata are retained. Raw inputs never enter an
  // activity record.
  PrivacyActivityPrivateInputType private_input_type =
      PrivacyActivityPrivateInputType::kNone;
  uint32_t private_input_count = 0;

  PrivacyActivityDisclosureType disclosed_type =
      PrivacyActivityDisclosureType::kNone;
  uint32_t disclosed_field_count = 0;

  bool private_compute_attempted = false;
  bool private_compute_succeeded = false;
  std::string reason;
};

}  // namespace privy

#endif  // CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_ACTIVITY_H_
