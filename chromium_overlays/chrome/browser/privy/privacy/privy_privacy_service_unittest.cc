#include "chrome/browser/privy/privacy/privy_privacy_service.h"

#include <memory>
#include <optional>
#include <utility>

#include "base/functional/bind.h"
#include "components/privy_privacy/compute/deterministic_private_compute_provider.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace privy {
namespace {

struct EvaluationResult {
  PrivacyDecision decision;
  std::optional<PrivateComputeResult> compute_result;
};

class FailingPrivateComputeProvider final : public PrivateComputeProvider {
 public:
  bool Supports(PrivateComputeCapability capability) const override {
    return capability == PrivateComputeCapability::kFrequencyCap;
  }

  void Execute(PrivateComputeRequest request,
               ExecuteCallback callback) override {
    PrivateComputeResult result;
    result.error = "intentional_test_failure";
    std::move(callback).Run(std::move(result));
  }
};

PrivacyRequest FrequencyPrivacyRequest() {
  PrivacyRequest request;
  request.requester = url::Origin::Create(GURL("https://publisher.test"));
  request.requester_type = RequesterType::kSite;
  request.surface = PrivacySurface::kFrequencyCap;
  request.contains_behavioral_data = true;
  return request;
}

PrivateComputeRequest FrequencyComputeRequest(uint32_t prior,
                                              uint32_t maximum) {
  PrivateComputeRequest request;
  request.requester = url::Origin::Create(GURL("https://publisher.test"));
  request.capability = PrivateComputeCapability::kFrequencyCap;
  request.input = FrequencyCapInput{
      .campaign_commitment = 42,
      .prior_impressions = prior,
      .maximum_impressions = maximum,
  };
  return request;
}

EvaluationResult EvaluateSync(
    PrivyPrivacyService* service,
    PrivacyRequest request,
    std::optional<PrivateComputeRequest> compute_request) {
  std::optional<EvaluationResult> result;
  service->EvaluateAndMaybeCompute(
      std::move(request), std::move(compute_request),
      base::BindOnce(
          [](std::optional<EvaluationResult>* out, PrivacyDecision decision,
             std::optional<PrivateComputeResult> compute_result) {
            *out = EvaluationResult{std::move(decision),
                                    std::move(compute_result)};
          },
          &result));

  if (!result) {
    ADD_FAILURE() << "PrivyPrivacyService did not complete synchronously in test";
    return EvaluationResult{
        PrivacyDecision{PrivacyAction::kBlock, "missing_callback"},
        std::nullopt};
  }

  return std::move(*result);
}

TEST(PrivyPrivacyServiceTest, ComputesBehavioralDecisionWithoutRawFallback) {
  PrivyPrivacyService service(
      std::make_unique<DeterministicPrivateComputeProvider>(), false);

  EvaluationResult result = EvaluateSync(
      &service, FrequencyPrivacyRequest(), FrequencyComputeRequest(2, 3));

  EXPECT_EQ(result.decision.action, PrivacyAction::kComputePrivately);
  ASSERT_TRUE(result.compute_result.has_value());
  ASSERT_TRUE(result.compute_result->success);
  ASSERT_TRUE(std::holds_alternative<bool>(result.compute_result->value));
  EXPECT_TRUE(std::get<bool>(result.compute_result->value));
}

TEST(PrivyPrivacyServiceTest, MissingComputeRequestFailsClosed) {
  PrivyPrivacyService service(
      std::make_unique<DeterministicPrivateComputeProvider>(), false);

  EvaluationResult result =
      EvaluateSync(&service, FrequencyPrivacyRequest(), std::nullopt);

  EXPECT_EQ(result.decision.action, PrivacyAction::kBlock);
  EXPECT_FALSE(result.compute_result.has_value());
}

TEST(PrivyPrivacyServiceTest, ProviderFailureFailsClosed) {
  PrivyPrivacyService service(std::make_unique<FailingPrivateComputeProvider>(),
                              false);

  EvaluationResult result = EvaluateSync(
      &service, FrequencyPrivacyRequest(), FrequencyComputeRequest(2, 3));

  EXPECT_EQ(result.decision.action, PrivacyAction::kBlock);
  ASSERT_TRUE(result.compute_result.has_value());
  EXPECT_FALSE(result.compute_result->success);
  EXPECT_EQ(result.decision.reason, "private_compute_failed");
}

TEST(PrivyPrivacyServiceTest, OffTheRecordStateIsExplicit) {
  PrivyPrivacyService service(
      std::make_unique<DeterministicPrivateComputeProvider>(), true);

  EXPECT_TRUE(service.off_the_record());
}

}  // namespace
}  // namespace privy
