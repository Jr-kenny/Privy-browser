#include "components/privy_privacy/core/privacy_policy_engine.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace privy {
namespace {

PrivacyRequest BaseSiteRequest(PrivacySurface surface) {
  PrivacyRequest request;
  request.requester = url::Origin::Create(GURL("https://example.test"));
  request.requester_type = RequesterType::kSite;
  request.surface = surface;
  return request;
}

TEST(PrivacyPolicyEngineTest, UsesPrivateComputeForBehavioralTelemetry) {
  PrivacyPolicyEngine engine;
  auto request = BaseSiteRequest(PrivacySurface::kTelemetry);
  request.contains_behavioral_data = true;
  request.supports_private_compute = true;

  EXPECT_EQ(engine.Evaluate(request).action,
            PrivacyAction::kComputePrivately);
}

TEST(PrivacyPolicyEngineTest, FailsClosedWhenPrivateComputeUnavailable) {
  PrivacyPolicyEngine engine;
  auto request = BaseSiteRequest(PrivacySurface::kTelemetry);
  request.contains_behavioral_data = true;
  request.supports_private_compute = false;

  EXPECT_EQ(engine.Evaluate(request).action, PrivacyAction::kBlock);
}

TEST(PrivacyPolicyEngineTest, BlocksPersistentCrossSiteState) {
  PrivacyPolicyEngine engine;
  auto request = BaseSiteRequest(PrivacySurface::kCookie);
  request.cross_site = true;
  request.persistent = true;

  EXPECT_EQ(engine.Evaluate(request).action, PrivacyAction::kBlock);
}

TEST(PrivacyPolicyEngineTest, AllowsOrdinaryFirstPartySessionCookie) {
  PrivacyPolicyEngine engine;
  auto request = BaseSiteRequest(PrivacySurface::kCookie);
  request.cross_site = false;
  request.persistent = false;

  EXPECT_EQ(engine.Evaluate(request).action, PrivacyAction::kAllow);
}

TEST(PrivacyPolicyEngineTest, PromptsBeforeExtensionBehavioralEgress) {
  PrivacyPolicyEngine engine;
  auto request = BaseSiteRequest(PrivacySurface::kExtensionEgress);
  request.requester_type = RequesterType::kExtension;
  request.contains_behavioral_data = true;

  EXPECT_EQ(engine.Evaluate(request).action, PrivacyAction::kPrompt);
}

TEST(PrivacyPolicyEngineTest, SanitizesFingerprintingSurface) {
  PrivacyPolicyEngine engine;
  auto request = BaseSiteRequest(PrivacySurface::kFingerprinting);

  EXPECT_EQ(engine.Evaluate(request).action, PrivacyAction::kSanitize);
}

}  // namespace
}  // namespace privy
