#ifndef CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_SERVICE_FACTORY_H_
#define CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_SERVICE_FACTORY_H_

#include <memory>

#include "base/no_destructor.h"
#include "chrome/browser/profiles/profile_keyed_service_factory.h"

class KeyedService;
class Profile;

namespace content {
class BrowserContext;
}  // namespace content

namespace privy {

class PrivyPrivacyService;

class PrivyPrivacyServiceFactory : public ProfileKeyedServiceFactory {
 public:
  static PrivyPrivacyService* GetForProfile(Profile* profile);
  static PrivyPrivacyServiceFactory* GetInstance();

  PrivyPrivacyServiceFactory(const PrivyPrivacyServiceFactory&) = delete;
  PrivyPrivacyServiceFactory& operator=(const PrivyPrivacyServiceFactory&) =
      delete;

 private:
  friend base::NoDestructor<PrivyPrivacyServiceFactory>;

  PrivyPrivacyServiceFactory();
  ~PrivyPrivacyServiceFactory() override;

  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

}  // namespace privy

#endif  // CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_SERVICE_FACTORY_H_
