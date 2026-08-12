// Copyright 2026 The Privy Browser Authors

#include "chrome/browser/privy/privacy/privy_privacy_service_factory.h"

#include <optional>
#include <utility>

#include "base/functional/bind.h"
#include "chrome/browser/privy/privacy/privy_privacy_service.h"
#include "chrome/test/base/testing_profile.h"
#include "content/public/test/browser_task_environment.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace privy {
namespace {

class PrivyPrivacyServiceFactoryTest : public testing::Test {
 protected:
  content::BrowserTaskEnvironment task_environment_;
};

TEST_F(PrivyPrivacyServiceFactoryTest, CreatesDistinctRegularAndOtrServices) {
  TestingProfile profile;
  PrivyPrivacyService* regular_service =
      PrivyPrivacyServiceFactory::GetForProfile(&profile);
  ASSERT_NE(regular_service, nullptr);

  Profile* otr_profile = profile.GetPrimaryOTRProfile(
      /*create_if_needed=*/true);
  ASSERT_NE(otr_profile, nullptr);
  PrivyPrivacyService* otr_service =
      PrivyPrivacyServiceFactory::GetForProfile(otr_profile);

  ASSERT_NE(otr_service, nullptr);
  EXPECT_NE(regular_service, otr_service);
  EXPECT_FALSE(regular_service->off_the_record());
  EXPECT_TRUE(otr_service->off_the_record());
}

TEST_F(PrivyPrivacyServiceFactoryTest, OtrActivityDoesNotReuseRegularState) {
  TestingProfile profile;
  PrivyPrivacyService* regular_service =
      PrivyPrivacyServiceFactory::GetForProfile(&profile);
  Profile* otr_profile = profile.GetPrimaryOTRProfile(
      /*create_if_needed=*/true);
  PrivyPrivacyService* otr_service =
      PrivyPrivacyServiceFactory::GetForProfile(otr_profile);
  ASSERT_NE(regular_service, nullptr);
  ASSERT_NE(otr_service, nullptr);

  PrivacyRequest request;
  request.requester_type = RequesterType::kBrowser;
  request.surface = PrivacySurface::kTelemetry;
  regular_service->EvaluateAndMaybeCompute(
      std::move(request), std::nullopt,
      base::BindOnce([](PrivacyDecision,
                        std::optional<PrivateComputeResult>) {}));

  EXPECT_EQ(regular_service->GetRecentActivity().size(), 1u);
  EXPECT_TRUE(otr_service->GetRecentActivity().empty());
}

}  // namespace
}  // namespace privy
