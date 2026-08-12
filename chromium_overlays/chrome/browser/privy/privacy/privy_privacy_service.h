#ifndef CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_SERVICE_H_
#define CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_SERVICE_H_

#include <cstddef>
#include <deque>
#include <memory>
#include <optional>
#include <string>

#include "base/functional/callback_forward.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "chrome/browser/privy/privacy/privy_privacy_activity.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/privy_privacy/compute/private_compute_provider.h"
#include "components/privy_privacy/core/privacy_policy_engine.h"

namespace privy {

// Profile-scoped browser service that owns privacy policy decisions and the
// private-compute provider boundary. Site/extension integration code talks to
// this service instead of talking directly to Aleo or another provider.
class PrivyPrivacyService : public KeyedService {
 public:
  using EvaluateCallback = base::OnceCallback<void(
      PrivacyDecision,
      std::optional<PrivateComputeResult>)>;

  class Observer {
   public:
    virtual ~Observer() = default;

    virtual void OnPrivacyActivityAdded(const PrivacyActivity& activity) = 0;
    virtual void OnPrivacyActivityCleared() = 0;
  };

  static constexpr size_t kMaxActivityEntries = 100;

  PrivyPrivacyService(std::unique_ptr<PrivateComputeProvider> compute_provider,
                      bool off_the_record);
  PrivyPrivacyService(const PrivyPrivacyService&) = delete;
  PrivyPrivacyService& operator=(const PrivyPrivacyService&) = delete;
  ~PrivyPrivacyService() override;

  void EvaluateAndMaybeCompute(
      PrivacyRequest request,
      std::optional<PrivateComputeRequest> compute_request,
      EvaluateCallback callback);

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  const std::deque<PrivacyActivity>& GetRecentActivity() const {
    return activity_;
  }
  // Activity is profile-local memory. Callers that implement a browser-data
  // clear flow must invoke this explicitly when Privy activity should clear.
  void ClearActivity();
  size_t GetActivityCount(PrivacyAction action) const;

  bool off_the_record() const { return off_the_record_; }
  bool SupportsPrivateCompute(PrivateComputeCapability capability) const {
    return compute_provider_ && compute_provider_->Supports(capability);
  }
  const PrivateComputeProvider* compute_provider_for_testing() const {
    return compute_provider_.get();
  }

 private:
  void OnPrivateComputeComplete(PrivacyRequest request,
                                PrivacyDecision original_decision,
                                PrivateComputeCapability capability,
                                EvaluateCallback callback,
                                PrivateComputeResult result);
  void RecordActivity(
      const PrivacyRequest& request,
      PrivacyAction decision,
      const std::string& reason,
      std::optional<PrivateComputeCapability> capability,
      const PrivateComputeResult* result);

  PrivacyPolicyEngine policy_engine_;
  std::unique_ptr<PrivateComputeProvider> compute_provider_;
  const bool off_the_record_;
  std::deque<PrivacyActivity> activity_;
  base::ObserverList<Observer> observers_;

  base::WeakPtrFactory<PrivyPrivacyService> weak_factory_{this};
};

}  // namespace privy

#endif  // CHROME_BROWSER_PRIVY_PRIVACY_PRIVY_PRIVACY_SERVICE_H_
