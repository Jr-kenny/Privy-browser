#include "components/privy_privacy/compute/deterministic_private_compute_provider.h"

#include <utility>

#include "base/functional/callback.h"

namespace privy {
namespace {

PrivateComputeResult ErrorResult(const char* error) {
  PrivateComputeResult result;
  result.error = error;
  return result;
}

}  // namespace

bool DeterministicPrivateComputeProvider::Supports(
    PrivateComputeCapability capability) const {
  switch (capability) {
    case PrivateComputeCapability::kFrequencyCap:
    case PrivateComputeCapability::kConversionAttribution:
    case PrivateComputeCapability::kTelemetryBucket:
    case PrivateComputeCapability::kPersonalizationMatch:
      return true;
  }

  return false;
}

void DeterministicPrivateComputeProvider::Execute(
    PrivateComputeRequest request,
    ExecuteCallback callback) {
  PrivateComputeResult result;

  switch (request.capability) {
    case PrivateComputeCapability::kFrequencyCap: {
      const auto* input = std::get_if<FrequencyCapInput>(&request.input);
      if (!input) {
        std::move(callback).Run(ErrorResult("invalid_frequency_cap_input"));
        return;
      }

      result.success = true;
      result.value = input->prior_impressions < input->maximum_impressions;
      break;
    }

    case PrivateComputeCapability::kConversionAttribution: {
      const auto* input =
          std::get_if<ConversionAttributionInput>(&request.input);
      if (!input) {
        std::move(callback).Run(
            ErrorResult("invalid_conversion_attribution_input"));
        return;
      }

      result.success = true;
      result.value = input->converted && input->campaign_commitment != 0 &&
                     input->exposure_commitment != 0;
      break;
    }

    case PrivateComputeCapability::kTelemetryBucket: {
      const auto* input = std::get_if<TelemetryBucketInput>(&request.input);
      if (!input || input->bucket_width == 0) {
        std::move(callback).Run(ErrorResult("invalid_telemetry_bucket_input"));
        return;
      }

      result.success = true;
      result.value =
          (input->private_event_count / input->bucket_width) * input->bucket_width;
      break;
    }

    case PrivateComputeCapability::kPersonalizationMatch: {
      const auto* input =
          std::get_if<PersonalizationMatchInput>(&request.input);
      if (!input) {
        std::move(callback).Run(
            ErrorResult("invalid_personalization_match_input"));
        return;
      }

      result.success = true;
      result.value = input->requested_segment_commitment != 0 &&
                     input->requested_segment_commitment ==
                         input->local_segment_commitment;
      break;
    }
  }

  std::move(callback).Run(std::move(result));
}

}  // namespace privy
