#include "chrome/browser/privy/privacy/privy_privacy_service_factory.h"

#include <memory>

#include "chrome/browser/privy/privacy/privy_privacy_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_selections.h"
#include "components/privy_privacy/compute/deterministic_private_compute_provider.h"
#include "content/public/browser/browser_context.h"

namespace privy {

// static
PrivyPrivacyService* PrivyPrivacyServiceFactory::GetForProfile(
    Profile* profile) {
  return static_cast<PrivyPrivacyService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
PrivyPrivacyServiceFactory* PrivyPrivacyServiceFactory::GetInstance() {
  static base::NoDestructor<PrivyPrivacyServiceFactory> instance;
  return instance.get();
}

PrivyPrivacyServiceFactory::PrivyPrivacyServiceFactory()
    : ProfileKeyedServiceFactory(
          "PrivyPrivacyService",
          ProfileSelections::Builder()
              // Incognito gets an independent ephemeral service instance rather
              // than being redirected to the regular profile.
              .WithRegular(ProfileSelection::kOwnInstance)
              .WithGuest(ProfileSelection::kOwnInstance)
              .WithAshInternals(ProfileSelection::kOwnInstance)
              .Build()) {}

PrivyPrivacyServiceFactory::~PrivyPrivacyServiceFactory() = default;

std::unique_ptr<KeyedService>
PrivyPrivacyServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  Profile* profile = Profile::FromBrowserContext(context);
  return std::make_unique<PrivyPrivacyService>(
      std::make_unique<DeterministicPrivateComputeProvider>(),
      profile->IsOffTheRecord());
}

}  // namespace privy
