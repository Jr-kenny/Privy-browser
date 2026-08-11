#ifndef COMPONENTS_PRIVY_PRIVACY_COMPUTE_PRIVATE_COMPUTE_TYPES_H_
#define COMPONENTS_PRIVY_PRIVACY_COMPUTE_PRIVATE_COMPUTE_TYPES_H_

#include <cstdint>
#include <string>
#include <variant>

#include "url/origin.h"

namespace privy {

enum class PrivateComputeCapability {
  kFrequencyCap,
  kConversionAttribution,
  kTelemetryBucket,
  kPersonalizationMatch,
};

struct FrequencyCapInput {
  uint64_t campaign_commitment = 0;
  uint32_t prior_impressions = 0;
  uint32_t maximum_impressions = 0;
};

struct ConversionAttributionInput {
  uint64_t campaign_commitment = 0;
  uint64_t exposure_commitment = 0;
  bool converted = false;
};

struct TelemetryBucketInput {
  uint32_t private_event_count = 0;
  uint32_t bucket_width = 1;
};

struct PersonalizationMatchInput {
  uint64_t requested_segment_commitment = 0;
  uint64_t local_segment_commitment = 0;
};

using PrivateComputeInput =
    std::variant<FrequencyCapInput,
                 ConversionAttributionInput,
                 TelemetryBucketInput,
                 PersonalizationMatchInput>;

struct PrivateComputeRequest {
  url::Origin requester;
  PrivateComputeCapability capability = PrivateComputeCapability::kFrequencyCap;
  PrivateComputeInput input = FrequencyCapInput{};
};

using PrivateComputeValue = std::variant<bool, uint32_t>;

struct PrivateComputeResult {
  bool success = false;
  PrivateComputeValue value = false;

  // Provider-specific proof/transcript material. Core browser policy must not
  // parse this to make authorization decisions.
  std::string proof;
  std::string error;
};

}  // namespace privy

#endif  // COMPONENTS_PRIVY_PRIVACY_COMPUTE_PRIVATE_COMPUTE_TYPES_H_
