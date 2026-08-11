#include "components/privy_privacy/compute/deterministic_private_compute_provider.h"

#include <optional>
#include <utility>

#include "base/functional/bind.h"
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
  EXPECT_TRUE(result.has_value());
  return std::move(*result);
}

TEST(DeterministicPrivateComputeProviderTest, FrequencyCapAllowsBelowLimit) {
  DeterministicPrivateComputeProvider provider;
  PrivateComputeRequest request;
  request.capability = PrivateComputeCapability::kFrequencyCap;
  request.input = FrequencyCapInput{
      .campaign_commitment = 42,
      .prior_impressions = 2,
      .maximum_impressions = 3,
  };

  PrivateComputeResult result = ExecuteSync(&provider, std::move(request));

  ASSERT_TRUE(result.success);
  ASSERT_TRUE(std::holds_alternative<bool>(result.value));
  EXPECT_TRUE(std::get<bool>(result.value));
}

TEST(DeterministicPrivateComputeProviderTest, FrequencyCapBlocksAtLimit) {
  DeterministicPrivateComputeProvider provider;
  PrivateComputeRequest request;
  request.capability = PrivateComputeCapability::kFrequencyCap;
  request.input = FrequencyCapInput{
      .campaign_commitment = 42,
      .prior_impressions = 3,
      .maximum_impressions = 3,
  };

  PrivateComputeResult result = ExecuteSync(&provider, std::move(request));

  ASSERT_TRUE(result.success);
  EXPECT_FALSE(std::get<bool>(result.value));
}

TEST(DeterministicPrivateComputeProviderTest, RejectsMismatchedTypedInput) {
  DeterministicPrivateComputeProvider provider;
  PrivateComputeRequest request;
  request.capability = PrivateComputeCapability::kFrequencyCap;
  request.input = TelemetryBucketInput{
      .private_event_count = 8,
      .bucket_width = 5,
  };

  PrivateComputeResult result = ExecuteSync(&provider, std::move(request));

  EXPECT_FALSE(result.success);
  EXPECT_EQ(result.error, "invalid_frequency_cap_input");
}

TEST(DeterministicPrivateComputeProviderTest, TelemetryReturnsCoarseBucket) {
  DeterministicPrivateComputeProvider provider;
  PrivateComputeRequest request;
  request.capability = PrivateComputeCapability::kTelemetryBucket;
  request.input = TelemetryBucketInput{
      .private_event_count = 18,
      .bucket_width = 5,
  };

  PrivateComputeResult result = ExecuteSync(&provider, std::move(request));

  ASSERT_TRUE(result.success);
  ASSERT_TRUE(std::holds_alternative<uint32_t>(result.value));
  EXPECT_EQ(std::get<uint32_t>(result.value), 15u);
}

TEST(DeterministicPrivateComputeProviderTest, PersonalizationOnlyReturnsMatch) {
  DeterministicPrivateComputeProvider provider;
  PrivateComputeRequest request;
  request.capability = PrivateComputeCapability::kPersonalizationMatch;
  request.input = PersonalizationMatchInput{
      .requested_segment_commitment = 77,
      .local_segment_commitment = 77,
  };

  PrivateComputeResult result = ExecuteSync(&provider, std::move(request));

  ASSERT_TRUE(result.success);
  EXPECT_TRUE(std::get<bool>(result.value));
}

}  // namespace
}  // namespace privy
