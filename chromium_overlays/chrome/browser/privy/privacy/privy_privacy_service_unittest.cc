#include "chrome/browser/privy/privacy/privy_privacy_service.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
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

class RecordingObserver final : public PrivyPrivacyService::Observer {
 public:
  void OnPrivacyActivityAdded(const PrivacyActivity& activity) override {
    added_activity.push_back(activity);
  }

  void OnPrivacyActivityCleared() override { ++clear_count; }

  std::vector<PrivacyActivity> added_activity;
  size_t clear_count = 0;
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

TEST(PrivyPrivacyServiceTest, RecordsPrivateComputeActivityWithoutRawInput) {
  PrivyPrivacyService service(
      std::make_unique<DeterministicPrivateComputeProvider>(), false);
  RecordingObserver observer;
  service.AddObserver(&observer);

  EvaluationResult result = EvaluateSync(
      &service, FrequencyPrivacyRequest(), FrequencyComputeRequest(2, 3));

  ASSERT_EQ(service.GetRecentActivity().size(), 1u);
  const PrivacyActivity& activity = service.GetRecentActivity().front();
  EXPECT_FALSE(activity.timestamp.is_null());
  EXPECT_EQ(activity.requester.Serialize(), "https://publisher.test");
  EXPECT_EQ(activity.requester_type, RequesterType::kSite);
  EXPECT_EQ(activity.surface, PrivacySurface::kFrequencyCap);
  EXPECT_EQ(activity.decision, PrivacyAction::kComputePrivately);
  EXPECT_EQ(activity.private_input_type,
            PrivacyActivityPrivateInputType::kFrequencyCap);
  EXPECT_EQ(activity.private_input_count, 3u);
  EXPECT_TRUE(activity.private_compute_attempted);
  EXPECT_TRUE(activity.private_compute_succeeded);
  EXPECT_EQ(activity.disclosed_type,
            PrivacyActivityDisclosureType::kBooleanResult);
  EXPECT_EQ(activity.disclosed_field_count, 1u);
  EXPECT_EQ(activity.reason, "behavioral_data_has_private_compute_path");
  EXPECT_EQ(observer.added_activity.size(), 1u);
  ASSERT_TRUE(result.compute_result.has_value());
  EXPECT_TRUE(result.compute_result->success);

  service.ClearActivity();
  EXPECT_TRUE(service.GetRecentActivity().empty());
  EXPECT_EQ(observer.clear_count, 1u);
  service.RemoveObserver(&observer);
}

TEST(PrivyPrivacyServiceTest, RecordsAllowAndBlockActivity) {
  PrivyPrivacyService service(
      std::make_unique<DeterministicPrivateComputeProvider>(), false);

  PrivacyRequest browser_request = FrequencyPrivacyRequest();
  browser_request.requester_type = RequesterType::kBrowser;
  EvaluateSync(&service, std::move(browser_request), std::nullopt);

  EvaluateSync(&service, FrequencyPrivacyRequest(), std::nullopt);

  ASSERT_EQ(service.GetRecentActivity().size(), 2u);
  EXPECT_EQ(service.GetRecentActivity()[0].decision, PrivacyAction::kAllow);
  EXPECT_EQ(service.GetRecentActivity()[1].decision, PrivacyAction::kBlock);
  EXPECT_EQ(service.GetActivityCount(PrivacyAction::kAllow), 1u);
  EXPECT_EQ(service.GetActivityCount(PrivacyAction::kBlock), 1u);
}

TEST(PrivyPrivacyServiceTest, RecordsProviderFailureAsFailedPrivateCompute) {
  PrivyPrivacyService service(std::make_unique<FailingPrivateComputeProvider>(),
                              false);

  EvaluateSync(&service, FrequencyPrivacyRequest(),
               FrequencyComputeRequest(2, 3));

  ASSERT_EQ(service.GetRecentActivity().size(), 1u);
  const PrivacyActivity& activity = service.GetRecentActivity().front();
  EXPECT_EQ(activity.decision, PrivacyAction::kBlock);
  EXPECT_TRUE(activity.private_compute_attempted);
  EXPECT_FALSE(activity.private_compute_succeeded);
  EXPECT_EQ(activity.disclosed_type, PrivacyActivityDisclosureType::kNone);
  EXPECT_EQ(activity.disclosed_field_count, 0u);
  EXPECT_EQ(activity.reason, "private_compute_failed");
}

TEST(PrivyPrivacyServiceTest, ActivityIsBounded) {
  PrivyPrivacyService service(
      std::make_unique<DeterministicPrivateComputeProvider>(), false);

  for (size_t i = 0; i < PrivyPrivacyService::kMaxActivityEntries + 5; ++i) {
    PrivacyRequest request = FrequencyPrivacyRequest();
    request.requester_type = RequesterType::kBrowser;
    EvaluateSync(&service, std::move(request), std::nullopt);
  }

  EXPECT_EQ(service.GetRecentActivity().size(),
            PrivyPrivacyService::kMaxActivityEntries);
}

TEST(PrivyPrivacyServiceTest, OffTheRecordActivityIsIsolated) {
  PrivyPrivacyService regular_service(
      std::make_unique<DeterministicPrivateComputeProvider>(), false);
  PrivyPrivacyService off_the_record_service(
      std::make_unique<DeterministicPrivateComputeProvider>(), true);

  EvaluateSync(&regular_service, FrequencyPrivacyRequest(), std::nullopt);
  EXPECT_EQ(regular_service.GetRecentActivity().size(), 1u);
  EXPECT_TRUE(off_the_record_service.GetRecentActivity().empty());

  EvaluateSync(&off_the_record_service, FrequencyPrivacyRequest(),
               std::nullopt);
  EXPECT_EQ(regular_service.GetRecentActivity().size(), 1u);
  EXPECT_EQ(off_the_record_service.GetRecentActivity().size(), 1u);
}

}  // namespace
}  // namespace privy
