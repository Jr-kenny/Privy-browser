#include "components/privy_privacy/compute/deterministic_private_compute_provider.h"

#include <optional>
#include <utility>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace privy {
namespace {

PrivateComputeResult ExecuteSync(DeterministicPrivateComputeProvider* provider,
                                 PrivateComputeRequest request) {
  std::optional<PrivateComputeResult> result;
  provider->Execute(
      std::move(request),
      base::BindOnce(
          [](std::optional<PrivateComputeResult>* out,
             PrivateComputeResult value) { *out = std::move(value); },
          &result));

  if (!result) {
    ADD_FAILURE() << "PrivateComputeProvider did not complete synchronously in test";
    PrivateComputeResult fallback;
    fallback.error = "missing_callback";
    return fallback;
  }

  return std::move(*result);
}

TEST(DeterministicPrivateComputeProviderTest, FrequencyCapAllowsBelowLimit) {
  DeterministicPrivateComputeProvider provider;
  PrivateComputeRequest request;
  request.capability = PrivateComputeCapability::kFrequencyCap;
  FrequencyCapInput input;
  input.campaign_commitment = 42;
  input.prior_impressions = 2;
  input.maximum_impressions = 3;
  request.input = input;

  PrivateComputeResult result = ExecuteSync(&provider, std::move(request));

  ASSERT_TRUE(result.success);
  ASSERT_TRUE(std::holds_alternative<bool>(result.value));
  EXPECT_TRUE(std::get<bool>(result.value));
}

TEST(DeterministicPrivateComputeProviderTest, FrequencyCapBlocksAtLimit) {
  DeterministicPrivateComputeProvider provider;
  PrivateComputeRequest request;
  request.capability = PrivateComputeCapability::kFrequencyCap;
  FrequencyCapInput input;
  input.campaign_commitment = 42;
  input.prior_impressions = 3;
  input.maximum_impressions = 3;
  request.input = input;

  PrivateComputeResult result = ExecuteSync(&provider, std::move(request));

  ASSERT_TRUE(result.success);
  EXPECT_FALSE(std::get<bool>(result.value));
}

TEST(DeterministicPrivateComputeProviderTest, RejectsMismatchedTypedInput) {
  DeterministicPrivateComputeProvider provider;
  PrivateComputeRequest request;
  request.capability = PrivateComputeCapability::kFrequencyCap;
  TelemetryBucketInput input;
  input.private_event_count = 8;
  input.bucket_width = 5;
  request.input = input;

  PrivateComputeResult result = ExecuteSync(&provider, std::move(request));

  EXPECT_FALSE(result.success);
  EXPECT_EQ(result.error, "invalid_frequency_cap_input");
}

TEST(DeterministicPrivateComputeProviderTest, TelemetryReturnsCoarseBucket) {
  DeterministicPrivateComputeProvider provider;
  PrivateComputeRequest request;
  request.capability = PrivateComputeCapability::kTelemetryBucket;
  TelemetryBucketInput input;
  input.private_event_count = 18;
  input.bucket_width = 5;
  request.input = input;

  PrivateComputeResult result = ExecuteSync(&provider, std::move(request));

  ASSERT_TRUE(result.success);
  ASSERT_TRUE(std::holds_alternative<uint32_t>(result.value));
  EXPECT_EQ(std::get<uint32_t>(result.value), 15u);
}

TEST(DeterministicPrivateComputeProviderTest, PersonalizationOnlyReturnsMatch) {
  DeterministicPrivateComputeProvider provider;
  PrivateComputeRequest request;
  request.capability = PrivateComputeCapability::kPersonalizationMatch;
  PersonalizationMatchInput input;
  input.requested_segment_commitment = 77;
  input.local_segment_commitment = 77;
  request.input = input;

  PrivateComputeResult result = ExecuteSync(&provider, std::move(request));

  ASSERT_TRUE(result.success);
  EXPECT_TRUE(std::get<bool>(result.value));
}

}  // namespace
}  // namespace privy
